//
// Created by mortacious on 10/29/21.
//

#pragma once
#include <vtkSmartPointer.h>
#include <vtkMapper.h>
#include <vtkVector.h>
#include <vtkQuaternion.h>
#include <vtkMatrix4x4.h>
#include <vtkCallbackCommand.h>
#include <memory>
#include "vtkTileHierarchyModule.h" // For export macro

class vtkRenderer;
class vtkActor;
class vtkWindow;
class vtkCamera;
class vtkPotreeLoader;
class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;
class vtkTileHierarchyLoaderThread;

//class CheckVisibilityCallback;
//class ReRenderCallback;

class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyMapper : public vtkMapper
{
public:
    static vtkTileHierarchyMapper* New();
    vtkTypeMacro(vtkTileHierarchyMapper, vtkMapper);
    void PrintSelf(ostream& os, vtkIndent indent) override;


    void SetLoader(vtkPotreeLoader* loader);
    vtkPotreeLoader* GetLoader();

    void SetNumThreads(unsigned int num_threads);

    vtkGetMacro(NumThreads, unsigned int);

    vtkSetMacro(PointBudget, std::size_t);
    vtkGetMacro(PointBudget, std::size_t);

    vtkSetMacro(MinimumNodeSize, float);
    vtkGetMacro(MinimumNodeSize, float);

    /**
     * Standard method for rendering a mapper. This method will be
     * called by the actor.
     */
    void Render(vtkRenderer* ren, vtkActor* a) override;

    ///@{
    /**
     * Standard vtkProp method to get 3D bounds of a 3D prop
     */
    double* GetBounds() VTK_SIZEHINT(6) override;
    void GetBounds(double bounds[6]) override { this->Superclass::GetBounds(bounds); }

    void ReleaseGraphicsResources(vtkWindow* win) override;
protected:
    vtkTileHierarchyMapper();
    ~vtkTileHierarchyMapper() override = default;

    /**
     * Need to loop over the hierarchy to compute bounds
     */
    void ComputeBounds();

    void OnNodeLoaded();

    /**
     * Calculate the priority of displaying a node
     * @param node
     * @param camera
     * @return
     */
    float GetPriority(const vtkTileHierarchyNodePtr& node, vtkRenderer* ren) const;

    bool BoundsInitialized;
    std::atomic_bool ForceUpdate;
    std::size_t PointBudget;
    float MinimumNodeSize;

    unsigned int NumThreads;
    vtkSmartPointer<vtkPotreeLoader> Loader;
    //vtkSmartPointer<vtkRenderer> Renderer;
    std::unique_ptr<vtkTileHierarchyLoaderThread> LoaderThread;

    //vtkNew<CheckVisibilityCallback> CheckVisibilityObserver;
    //vtkNew<ReRenderCallback> ReRenderObserver;
private:
    void InitLoaderThread();
    //friend class CheckVisibilityCallback;
    //friend class ReRenderCallback;

    vtkTileHierarchyMapper(const vtkTileHierarchyMapper&) = delete;
    void operator=(const vtkTileHierarchyMapper&) = delete;
};