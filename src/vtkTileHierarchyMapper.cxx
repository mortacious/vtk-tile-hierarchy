//
// Created by mortacious on 10/29/21.
//

#include "vtkTileHierarchyMapper.h"
#include "vtkTileHierarchyNode.h"
#include "vtkTileHierarchyLoaderBase.h"
#include "priorityQueue.h"
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkMath.h>
#include <vtkCamera.h>
#include <vtkWindow.h>
#include <vtkExecutive.h>
#include <vtkRenderWindowInteractor.h>

enum FrustumCull
{
    INSIDE_FRUSTUM,
    INTERSECT_FRUSTUM,
    OUTSIDE_FRUSTUM
};

int intersect(vtkCamera* camera, const vtkBoundingBox& bbox, double aspect_ratio) {
    int result = INSIDE_FRUSTUM;
    double frustum[24];
    camera->GetFrustumPlanes(aspect_ratio, frustum);

    for(int i =0; i < 6; i++){
        // iterate all 6 planes
        double a = frustum[(i*4)];
        double b = frustum[(i*4)+1];
        double c = frustum[(i*4)+2];
        double d = frustum[(i*4)+3];

        //std::cout << i << ": " << a << "x + " << b << "y + " << c << "z + " << d << std::endl;

        //  Basic VFC algorithm
        const double* max_point = bbox.GetMaxPoint();
        const double* min_point = bbox.GetMinPoint();
        vtkVector3d center((max_point[0] - min_point[0]) / 2 + min_point[0],
                           (max_point[1] - min_point[1]) / 2 + min_point[1],
                           (max_point[2] - min_point[2]) / 2 + min_point[2]);
        vtkVector3d radius(std::abs (static_cast<double> (max_point[0] - center[0])),
                           std::abs (static_cast<double> (max_point[1] - center[1])),
                           std::abs (static_cast<double> (max_point[2] - center[2])));

        double m = (center[0] * a) + (center[1] * b) + (center[2] * c) + d;
        double n = (radius[0] * std::abs(a)) + (radius[1] * std::abs(b)) + (radius[2] * std::abs(c));

        if (m + n < 0){
            result = OUTSIDE_FRUSTUM;
            break;
        }

        if (m - n < 0)
        {
            result = INTERSECT_FRUSTUM;
        }
    }

    return result;
}

class ReRenderCallback: public vtkCallbackCommand {
public:
    static ReRenderCallback* New();
    vtkTypeMacro(ReRenderCallback, vtkCallbackCommand);
    // Here we Create a vtkCallbackCommand and reimplement it.
    void Execute(vtkObject* caller, unsigned long evId, void* calldata) override
    {
        // Note the use of reinterpret_cast to cast the caller to the expected type.
        if (vtkCommand::TimerEvent == evId && *reinterpret_cast<int*>(calldata) == TimerId) {
            if(Mapper->ForceUpdate && Mapper->Renderer) {
                Mapper->Renderer->GetRenderWindow()->Render(); // force a render
            }
        }
    }

    // Set pointers to any clientData or callData here.
    vtkTileHierarchyMapper* Mapper;
    int TimerId;
protected:
    ReRenderCallback(): Mapper(nullptr), TimerId(-1) {

    }
    ~ReRenderCallback() override = default;
private:
    ReRenderCallback(const ReRenderCallback&) = delete;
    void operator=(const ReRenderCallback&) = delete;
};

vtkStandardNewMacro(ReRenderCallback);

vtkStandardNewMacro(vtkTileHierarchyMapper);

vtkTileHierarchyMapper::vtkTileHierarchyMapper()
 : BoundsInitialized(false), ForceUpdate(false), PointBudget(1000000), MinimumNodeSize(30.f), UseTimer(false) {
    vtkMapper::SetStatic(true); // This mapper does not use the pipeline
    ReRenderObserver->Mapper = this;
}

void vtkTileHierarchyMapper::SetLoader(vtkTileHierarchyLoaderBase* loader) {
    Register(loader);
    Loader.TakeReference(loader);
    Loader->SetNodeLoadedCallBack([this]() {OnNodeLoaded();});
    BoundsInitialized = false;
}


vtkTileHierarchyLoaderBase* vtkTileHierarchyMapper::GetLoader() {
    return Loader;
}

void vtkTileHierarchyMapper::ComputeBounds() {
    vtkMath::UninitializeBounds(this->Bounds);
    Loader->GetBoundingBox().GetBounds(Bounds);
    BoundsInitialized = true;
}

void vtkTileHierarchyMapper::ReleaseGraphicsResources(vtkWindow* win)
{
    if(Loader) {
        Loader->Shutdown();
    }
}

double* vtkTileHierarchyMapper::GetBounds()
{
    if(!BoundsInitialized)
        this->ComputeBounds();
    return this->Bounds;
}

void vtkTileHierarchyMapper::PrintSelf(ostream &os, vtkIndent indent) {
    this->Superclass::PrintSelf(os, indent);
}

void vtkTileHierarchyMapper::OnNodeLoaded() {
    ForceUpdate = true;
}

void vtkTileHierarchyMapper::Render(vtkRenderer *ren, vtkActor *a) {
    if(!Loader->GetInitialized()) {
        Loader->Initialize();
    }
    auto loader_state = Loader->PreRender();
    ForceUpdate = false;

    if(UseTimer) {
        if (!Renderer || Renderer.Get() != ren) {
            if (Renderer) {
                Renderer->GetRenderWindow()->GetInteractor()->RemoveObserver(ReRenderObserver);
            }
            Renderer = vtkSmartPointer<vtkRenderer>(ren);
            Renderer->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::TimerEvent, ReRenderObserver);
            ReRenderObserver->TimerId = Renderer->GetRenderWindow()->GetInteractor()->CreateRepeatingTimer(
                    100); // Check if nodes have been loaded 4 times a second
        }
    } else if(Renderer) {
        Renderer->GetRenderWindow()->GetInteractor()->RemoveObserver(ReRenderObserver);
        Renderer->GetRenderWindow()->GetInteractor()->DestroyTimer(ReRenderObserver->TimerId);
        Renderer = nullptr;
        ReRenderObserver->TimerId = -1;
    }

    if(ren->GetRenderWindow()->GetActualSize()[1] < 10) {
        return; // Do not render very small screens
    }
    PriorityQueue<vtkTileHierarchyNodePtr , float> process_queue;
    auto root_node = Loader->GetRootNode();
    process_queue.push(root_node, 0);
    std::size_t remaining_points = PointBudget;
    auto cam = ren->GetActiveCamera();
    double aspect_ratio = ren->GetTiledAspectRatio();

    while(!process_queue.empty()) {
        auto element = process_queue.top();
        auto node = std::get<0>(element);
        auto node_prio = std::get<1>(element);
        process_queue.pop();
        if(intersect(cam, node->GetBoundingBox(), aspect_ratio) != OUTSIDE_FRUSTUM) {
            if(node->IsLoaded()) {
                if(node->GetSize() <= remaining_points) {
                    remaining_points -= node->GetSize();
                    node->Render(ren, a); // Render this node!
                    for(auto& child: node->Children) {
                        if(child) {
                            float p = GetPriority(node, ren);
                            if(p > 0) {
                                process_queue.push(child, p);
                            } else if(child->IsLoaded()) {
                                Loader->UnloadNode(child, true); // remove this child and all nodes below because they are too small
                            }
                        }
                    }
                } else {
                    Loader->UnloadNode(node, true); // remove this node and all below (they will be cached)
                }
            } else {
                Loader->ScheduleForLoading(node, node_prio);
            }
        } else {
            Loader->UnloadNode(node, true); // remove this node and all below (they will be cached)

        }
    }
    Loader->PostRender(std::move(loader_state));
}


float vtkTileHierarchyMapper::GetPriority(const vtkTileHierarchyNodePtr &node, vtkRenderer* ren) const {
    auto cam = ren->GetActiveCamera();

    auto bbox = node->GetBoundingBox();
    vtkVector3d center;
    bbox.GetCenter(center.GetData());
    vtkVector3d camera_position;
    cam->GetPosition(camera_position.GetData());

    vtkVector3d half_size;
    bbox.GetLengths(half_size.GetData());
    half_size[0] /= 2;
    half_size[1] /= 2;
    half_size[2] /= 2;
    float bounding_radius = half_size.Norm();
    float projected_size;

    if(cam->GetParallelProjection()) {
        projected_size = bounding_radius;
    } else {
        float slope = std::tan(vtkMath::DegreesFromRadians(cam->GetViewAngle()));
        vtkVector3d center_pos_difference;
        vtkMath::Subtract(center.GetData(), camera_position.GetData(), center_pos_difference.GetData());
        float distance = center_pos_difference.Norm();
        projected_size = 0.5f * ren->GetRenderWindow()->GetActualSize()[1] * bounding_radius / (slope * distance);
        if(projected_size < MinimumNodeSize) {
            return -1; // ignore this node
        }
    }
    vtkVector3d cam_forward;
    cam->GetDirectionOfProjection(cam_forward.GetData());
    vtkVector3d center_pos_difference;
    vtkMath::Subtract(center.GetData(), camera_position.GetData(), center_pos_difference.GetData());
    vtkVector3d cam_to_node = center_pos_difference.Normalized();
    float angle = std::acos(cam_forward[0] * cam_to_node[0] + cam_forward[1] * cam_to_node[1] + cam_forward[2] * cam_to_node[2]);
    float angle_weight = std::abs(angle) + 1;
    return projected_size / angle_weight;
}




