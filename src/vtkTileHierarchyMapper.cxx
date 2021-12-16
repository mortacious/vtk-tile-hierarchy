//
// Created by mortacious on 10/29/21.
//

#include "vtkTileHierarchyMapper.h"
#include "vtkTileHierarchyNode.h"
#include "vtkPotreeLoader.h"
#include "vtkTileHierarchyLoaderThread.h"
#include "priorityQueue.h"
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkMath.h>
#include <vtkCamera.h>
#include <vtkWindow.h>
#include <vtkExecutive.h>
#include <vtkRenderWindowInteractor.h>


bool is_different(const vtkMatrix4x4* first, const vtkMatrix4x4* second) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (first->GetElement(i, j) != second->GetElement(i, j)) return true;
    return false;
}

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

class CheckVisibilityCallback: public vtkCallbackCommand {
public:
    static CheckVisibilityCallback* New();
    vtkTypeMacro(CheckVisibilityCallback, vtkCallbackCommand);

    // Here we Create a vtkCallbackCommand and reimplement it.
    void Execute(vtkObject* caller, unsigned long evId, void*) override
    {
        std::cout << "Camera moved" << std::endl;
        // Note the use of reinterpret_cast to cast the caller to the expected type.
        auto camera = Mapper->Renderer->GetActiveCamera();//reinterpret_cast<vtkCamera*>(caller);
        vtkVector3d camera_position;
        camera->GetPosition(camera_position.GetData());
        vtkQuaterniond camera_orientation(camera->GetOrientationWXYZ());
        vtkSmartPointer<vtkMatrix4x4> projection_matrix;
        projection_matrix.TakeReference(camera->GetProjectionTransformMatrix(Mapper->Renderer));

        vtkVector3d camera_position_difference;
        vtkMath::Subtract(camera_position.GetData(), LastPosition.GetData(), camera_position_difference.GetData());
        bool projection_different = true;
        if(LastProjection) {
            projection_different = is_different(projection_matrix, LastProjection);
        }
        if (!projection_different
            && camera_position_difference.Norm() < 0.01f
            && (camera_orientation - LastOrientation).Norm() < 0.01f)
            return;
        LastPosition = camera_position;
        LastOrientation = camera_orientation;
        LastProjection = projection_matrix;
        if(Mapper->Renderer)
            //std::cout << "Re-Rendering due to moved camera" << std::endl;
            Mapper->Renderer->GetRenderWindow()->Render(); // force a render
    }
    CheckVisibilityCallback(): Mapper(nullptr)
    {
    }

    // Set pointers to any clientData or callData here.
    vtkTileHierarchyMapper* Mapper;
    vtkVector3d LastPosition;
    vtkQuaterniond LastOrientation;
    vtkSmartPointer<vtkMatrix4x4> LastProjection;
};

class ReRenderCallback: public vtkCallbackCommand {
public:
    static ReRenderCallback* New();
    vtkTypeMacro(ReRenderCallback, vtkCallbackCommand);

    // Here we Create a vtkCallbackCommand and reimplement it.
    void Execute(vtkObject* caller, unsigned long evId, void*) override
    {
        // Note the use of reinterpret_cast to cast the caller to the expected type.
        if (vtkCommand::TimerEvent == evId) {
            if(Mapper->ForceUpdate && Mapper->Renderer) {
                std::cout << "Re-rendering " << count++ << std::endl;
                Mapper->Renderer->GetRenderWindow()->Render(); // force a render
            }
        }
    }
    ReRenderCallback(): Mapper(nullptr)
    {
    }

    // Set pointers to any clientData or callData here.
    vtkTileHierarchyMapper* Mapper;
    size_t count = 0;
};


vtkStandardNewMacro(CheckVisibilityCallback);
vtkStandardNewMacro(ReRenderCallback);



vtkStandardNewMacro(vtkTileHierarchyMapper);

vtkTileHierarchyMapper::vtkTileHierarchyMapper()
 : BoundsInitialized(false), ForceUpdate(false), Renderer(nullptr), PointBudget(1000000), MinimumNodeSize(30.f) {
    SetStatic(true); // This mapper does not use the pipeline
    CheckVisibilityObserver->Mapper = this;
    ReRenderObserver->Mapper = this;
}

void vtkTileHierarchyMapper::SetLoader(vtkPotreeLoader *loader) {
    Loader.TakeReference(loader);
    LoaderThread = std::make_shared<vtkTileHierarchyLoaderThread>(Loader.GetPointer());
    LoaderThread->SetNodeLoadedCallBack([this]() {OnNodeLoaded();});
    BoundsInitialized = false;
}

vtkPotreeLoader * vtkTileHierarchyMapper::GetLoader() {
    return Loader;
}

void vtkTileHierarchyMapper::ComputeBounds() {
    vtkMath::UninitializeBounds(this->Bounds);
    Loader->GetRootNode()->GetBoundingBox().GetBounds(Bounds);
    BoundsInitialized = true;
}

void vtkTileHierarchyMapper::ReleaseGraphicsResources(vtkWindow* win)
{
    //auto root_node = Loader->GetRootNode();
    //Loader->UnloadNode(root_node, true);
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
    std::cout << "Rendering" << std::endl;
    ForceUpdate = false;

    if(!Renderer || Renderer.Get() != ren) {
        if(Renderer) {
            Renderer->GetRenderWindow()->GetInteractor()->RemoveObserver(ReRenderObserver);
        }
        Renderer.TakeReference(ren);
        //Renderer->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::TimerEvent, ReRenderObserver);
        //Renderer->GetRenderWindow()->GetInteractor()->CreateRepeatingTimer(250); // Check if nodes have been loaded 10 times a second
        //Renderer->GetRenderWindow()->GetInteractor()->AddObserver(vtkCommand::EndInteractionEvent, CheckVisibilityObserver);
    }

    if(Renderer->GetRenderWindow()->GetActualSize()[1] < 10) {
        return; // Do not render very small screens
    }

    PriorityQueue<vtkTileHierarchyNodePtr , float> process_queue;
    auto root_node = Loader->GetRootNode();
    process_queue.push(root_node, 0);
    std::size_t remaining_points = PointBudget;
    auto cam = Renderer->GetActiveCamera();
    double aspect_ratio = ren->GetTiledAspectRatio();

    while(!process_queue.empty()) {
        auto node = process_queue.top();
        process_queue.pop();
        if(intersect(cam, node->GetBoundingBox(), aspect_ratio) != OUTSIDE_FRUSTUM) {
            if(node->IsLoaded()) {
                if(node->GetSize() <= remaining_points) {
                    remaining_points -= node->GetSize();
                    //std::cout << "Rendering node r" << node->GetName() << " with " << node->GetSize() << " points. Budget remaining: " << remaining_points << std::endl;
                    node->Render(ren, a); // Render this node!
                    for(auto& child: node->Children) {
                        if(child) {
                            float p = GetPriority(node, cam);
                            if(p > 0) {
                                process_queue.push(child, p);
                            } else if(child->IsLoaded()) {
                                //std::cout << "Unloading node " << child->GetName() << " with " << child->GetSize() << " points due to low visibility of " << p << std::endl;
                                Loader->UnloadNode(child, true); // remove this child and all nodes below because they are too small
                            }
                        }
                    }
                } else {
                    //std::cout << "Unloading node r" << node->GetName() << " with " << node->GetSize() << " points due to point budget" << std::endl;
                    Loader->UnloadNode(node, true); // remove this node and all below (they will be cached)
                }
            } else {
                //std::cout << "Scheduling node r" << node->GetName() << " for loading" << std::endl;
                LoaderThread->ScheduleForLoading(node);
            }
        } else {
            //std::cout << "Unloading node r" << node->GetName() << " with " << node->GetSize() << " points because it is outside the frustum" << std::endl;
            Loader->UnloadNode(node, true); // remove this node and all below (they will be cached)

        }
    }
    //std::cout << "--------------------------------------Rendering Done" << std::endl;
}


float vtkTileHierarchyMapper::GetPriority(const vtkTileHierarchyNodePtr &node, vtkCamera* camera) const {
    auto bbox = node->GetBoundingBox();
    vtkVector3d center;
    bbox.GetCenter(center.GetData());
    vtkVector3d camera_position;
    camera->GetPosition(camera_position.GetData());

    vtkVector3d half_size;
    bbox.GetLengths(half_size.GetData());
    half_size[0] /= 2;
    half_size[1] /= 2;
    half_size[2] /= 2;
    float bounding_radius = half_size.Norm();
    float projected_size;

    if(camera->GetParallelProjection()) {
        projected_size = bounding_radius;
    } else {
        float slope = std::tan(vtkMath::DegreesFromRadians(camera->GetViewAngle()));
        vtkVector3d center_pos_difference;
        vtkMath::Subtract(center.GetData(), camera_position.GetData(), center_pos_difference.GetData());
        float distance = center_pos_difference.Norm();
        projected_size = 0.5f * Renderer->GetRenderWindow()->GetActualSize()[1] * bounding_radius / (slope * distance);
        if(projected_size < MinimumNodeSize) {
            return -1; // ignore this node
        }
    }
    vtkVector3d cam_forward;
    camera->GetDirectionOfProjection(cam_forward.GetData());
    vtkVector3d center_pos_difference;
    vtkMath::Subtract(center.GetData(), camera_position.GetData(), center_pos_difference.GetData());
    vtkVector3d cam_to_node = center_pos_difference.Normalized();
    float angle = std::acos(cam_forward[0] * cam_to_node[0] + cam_forward[1] * cam_to_node[1] + cam_forward[2] * cam_to_node[2]);
    float angle_weight = std::abs(angle) + 1;
    return projected_size / angle_weight;
}




