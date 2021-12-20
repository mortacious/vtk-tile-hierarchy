//
// Created by mortacious on 10/31/21.
//

#include "vtkTileHierarchyLoaderThread.h"
#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyNode.h"

vtkTileHierarchyLoaderThread::vtkTileHierarchyLoaderThread(vtkSmartPointer<vtkTileHierarchyLoader> loader, unsigned int num_threads)
    : Running(true), Func(nullptr), Mutex(), MaxInQueue(2), Threads() {
    std::cout << "Spawning " << num_threads << " worker threads" << std::endl;
    Threads.reserve(num_threads);
    for(int i=0; i<num_threads; ++i) {
        Threads.emplace_back(&vtkTileHierarchyLoaderThread::Run, this);
    }
    Loader = loader;
}

vtkTileHierarchyLoaderThread::~vtkTileHierarchyLoaderThread() {
    std::unique_lock<std::mutex> lock{Mutex};
    Running = false;
    Cond.notify_all();
    lock.unlock();
    for(auto& thread: Threads) {
        if(thread.joinable()) thread.join();
    }
}

void vtkTileHierarchyLoaderThread::UnscheduleAll() {
    std::lock_guard<std::mutex> lock{Mutex};
    while(!NeedToLoad.empty()) {
        NeedToLoad.pop();
    }
}

void vtkTileHierarchyLoaderThread::SetNodeLoadedCallBack(const std::function<void()> &func) {
    Func = func;
}

void vtkTileHierarchyLoaderThread::ScheduleForLoading(vtkTileHierarchyNodePtr& node, float priority) {
    if(Loader->TryGetNodeFromCache(node)) {
        if(Func)
            Func();
    } else {
        std::lock_guard<std::mutex> lock{Mutex};

        NeedToLoad.push(std::make_pair(node, priority));
        while(NeedToLoad.size() > MaxInQueue) {
            NeedToLoad.popMin(); // Remove the smallest Element
        }
        Cond.notify_one();
    }
}

void vtkTileHierarchyLoaderThread::Run() {

    while(true) {
        std::unique_lock<std::mutex> lock{Mutex};
        if(NeedToLoad.empty()) {
            Cond.wait(lock, [&](){return !NeedToLoad.empty() || !Running;});
        }
        if(!Running)
            return;

        auto node = NeedToLoad.popMax().first;

        if(node->IsLoaded()) {
            continue; // skip already loaded nodes
        }

        lock.unlock();
        Loader->LoadNode(node);

        if(Func)
            Func();
    }
}
