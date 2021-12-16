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
#include <deque>
#include "minMaxHeap.h"
#include "vtkTileHierarchyModule.h" // For export macro


class vtkTileHierarchyLoader;
class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = std::shared_ptr<vtkTileHierarchyNode>;

VTK_WRAPEXCLUDE class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyLoaderThread {
public:
    explicit vtkTileHierarchyLoaderThread(vtkTileHierarchyLoader* loader);
    ~vtkTileHierarchyLoaderThread();
    void UnscheduleAll();
    void ScheduleForLoading(vtkTileHierarchyNodePtr& node, float priority);
    void SetNodeLoadedCallBack(const std::function<void()>& func);
private:
    void Run();

    vtkSmartPointer<vtkTileHierarchyLoader> Loader;
    bool Running;

    std::function<void()> Func;
    std::mutex Mutex;
    std::condition_variable Cond;

    using HeapElement = std::pair<vtkTileHierarchyNodePtr, float>;

    struct Compare
    {
        bool operator()(const HeapElement& e1, const HeapElement& e2) const
        {
            return e1.second < e2.second;
        }
    };
    minmax::MinMaxHeap<HeapElement, std::vector<HeapElement>, Compare> NeedToLoad;
    //PriorityQueue<vtkTileHierarchyNodePtr, float> NeedToLoad;
    //std::deque<vtkTileHierarchyNodePtr> NeedToLoad;
    unsigned int MaxInQueue;
    std::thread Thread;

};

