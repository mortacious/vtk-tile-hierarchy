//
// Created by mortacious on 10/31/21.
//

#include "vtkTileHierarchyLoaderThread.h"
#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyNode.h"

vtkTileHierarchyLoaderThread::vtkTileHierarchyLoaderThread(vtkTileHierarchyLoader *loader)
    : Running(true), Func(nullptr), Mutex(), Thread([this] { Run(); }) {
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

void vtkTileHierarchyLoaderThread::ScheduleForLoading(vtkTileHierarchyNodePtr& node) {
    if(Loader->IsCached(node)) {
        //std::cout << "Returning cached node r" << node->GetName() << std::endl;
        Loader->LoadNode(node); // This will be very quick
        if(Func)
            Func();
    } else {
        std::lock_guard<std::mutex> lock{Mutex};
        //std::cout << "Scheduling load from disk for node r" << node->GetName() << std::endl;
        NeedToLoad.push(node);
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

        //std::cout << "Checking for new node to load " << std::endl;
        auto node = NeedToLoad.front();
        NeedToLoad.pop();
        if(node->IsLoaded()) {
            //std::cout << "Node r" << node->GetName() << "Is already loaded." << std::endl;
            continue; // skip already loaded nodes
        }

        lock.unlock();
        //std::cout << "Loading node r" << node->GetName() << std::endl;
        Loader->LoadNode(node);
        //std::cout << "Loaded node r" << node->GetName() << " with " << node->GetSize() << "points" << std::endl;

        if(Func)
            Func();
        lock.lock();
    }
}
