//
// Created by mortacious on 10/31/21.
//

#include "vtkTileHierarchyLoaderThread.h"
#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyNode.h"

vtkTileHierarchyLoaderThread::vtkTileHierarchyLoaderThread(vtkTileHierarchyLoader *loader)
    : Running(true), Func(nullptr), Mutex(), MaxInQueue(2), Thread([this] { Run(); }) {
    Loader.TakeReference(loader);
}

vtkTileHierarchyLoaderThread::~vtkTileHierarchyLoaderThread() {
    std::unique_lock<std::mutex> lock{Mutex};
    Running = false;
    Cond.notify_all();
    lock.unlock();
    if(Thread.joinable()) Thread.join();
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
        if(NeedToLoad.size() == MaxInQueue) {
            NeedToLoad.popMin(); // Remove the smallest Element
        }
        NeedToLoad.push(std::make_pair(node, priority));
        Cond.notify_one();
    }
}

void vtkTileHierarchyLoaderThread::Run() {
    std::unique_lock<std::mutex> lock{Mutex};
    while(Running) {
        while(NeedToLoad.empty()) {
            Cond.wait(lock);
            if(!Running)
                return;
        }

        auto node = NeedToLoad.popMax().first;
        if(node->IsLoaded()) {
            continue; // skip already loaded nodes
        }

        lock.unlock();
        Loader->LoadNode(node);

        if(Func)
            Func();
        lock.lock();
    }
}
