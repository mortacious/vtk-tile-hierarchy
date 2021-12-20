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
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;


VTK_WRAPEXCLUDE class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyLoaderThread {
public:
    explicit vtkTileHierarchyLoaderThread(vtkSmartPointer<vtkTileHierarchyLoader> loader, unsigned int num_threads=2);
    ~vtkTileHierarchyLoaderThread();
    void UnscheduleAll();
    void ScheduleForLoading(vtkTileHierarchyNodePtr& node, float priority);
    void SetNodeLoadedCallBack(const std::function<void()>& func);
private:
    void Run();

    vtkSmartPointer<vtkTileHierarchyLoader> Loader;
    std::atomic_bool Running;

    std::function<void()> Func;
    mutable std::mutex Mutex;
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

    unsigned int MaxInQueue;
    std::vector<std::thread> Threads;

};

