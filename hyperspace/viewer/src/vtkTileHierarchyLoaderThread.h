//
// Created by mortacious on 10/31/21.
//

#pragma once
#include <vtkSmartPointer.h>
#include <vtkWrappingHints.h>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <queue>
#include "vtkHyperspaceExtensionsModule.h" // For export macro


class vtkTileHierarchyLoader;
class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = std::shared_ptr<vtkTileHierarchyNode>;

VTK_WRAPEXCLUDE class VTKHYPERSPACEEXTENSIONS_EXPORT vtkTileHierarchyLoaderThread {
public:
    explicit vtkTileHierarchyLoaderThread(vtkTileHierarchyLoader* loader);
    ~vtkTileHierarchyLoaderThread();
    void UnscheduleAll();
    void ScheduleForLoading(vtkTileHierarchyNodePtr& node);
    void SetNodeLoadedCallBack(const std::function<void()>& func);
private:
    void Run();

    vtkSmartPointer<vtkTileHierarchyLoader> Loader;
    bool Running;

    std::function<void()> Func;
    std::mutex Mutex;
    std::condition_variable Cond;
    std::queue<vtkTileHierarchyNodePtr> NeedToLoad;
    std::thread Thread;

};

