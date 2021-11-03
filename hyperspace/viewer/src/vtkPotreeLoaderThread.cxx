//
// Created by mortacious on 10/31/21.
//

#include "vtkPotreeLoaderThread.h"
#include "vtkPotreeLoader.h"
#include "vtkPotreeNode.h"

vtkPotreeLoaderThread::vtkPotreeLoaderThread(vtkPotreeLoader *loader)
    : Running(true), Func(nullptr), Mutex(), Thread([this] { Run(); }) {
    Loader.TakeReference(loader);
}

vtkPotreeLoaderThread::~vtkPotreeLoaderThread() {
    std::unique_lock<std::mutex> lock{Mutex};
    Running = false;
    Cond.notify_all();
    lock.unlock();
    if(Thread.joinable()) Thread.join();
}

void vtkPotreeLoaderThread::UnscheduleAll() {
    std::lock_guard<std::mutex> lock{Mutex};
    while(!NeedToLoad.empty()) {
        NeedToLoad.pop();
    }
}

void vtkPotreeLoaderThread::SetNodeLoadedCallBack(const std::function<void()> &func) {
    Func = func;
}

void vtkPotreeLoaderThread::ScheduleForLoading(vtkPotreeNodePtr& node) {
    if(Loader->IsCached(node)) {
        std::cout << "Returning cached node r" << node->GetName() << std::endl;
        Loader->LoadNode(node); // This will be very quick
        if(Func)
            Func();
    } else {
        std::lock_guard<std::mutex> lock{Mutex};
        std::cout << "Scheduling load from disk for node r" << node->GetName() << std::endl;
        NeedToLoad.push(node);
        Cond.notify_one();
    }
}

void vtkPotreeLoaderThread::Run() {
    std::cout << "Loader thread running" << std::endl;
    std::unique_lock<std::mutex> lock{Mutex};
    while(Running) {
        while(NeedToLoad.empty()) {
            //std::cout << "Loader thread waiting" << std::endl;
            Cond.wait(lock);
            if(!Running)
                return;
        }

        //std::cout << "Checking for new node to load " << std::endl;
        auto node = NeedToLoad.front();
        NeedToLoad.pop();
        if(node->IsLoaded()) continue; // skip already loaded nodes

        lock.unlock();
        Loader->LoadNode(node);
        std::cout << "Loaded node r" << node->GetName() << " with " << node->GetPointCount() << "points" << std::endl;

        if(Func)
            Func();
        lock.lock();
    }
}
