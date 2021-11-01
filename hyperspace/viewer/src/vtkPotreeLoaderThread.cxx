//
// Created by mortacious on 10/31/21.
//

#include "vtkPotreeLoaderThread.h"
#include "vtkPotreeLoader.h"
#include "vtkPotreeNode.h"

vtkPotreeLoaderThread::vtkPotreeLoaderThread(const vtkPotreeLoader *loader)
    : Loader(loader),
      Running(true),
      Thread(std::bind(&vtkPotreeLoaderThread::Run, this)) {}

vtkPotreeLoaderThread::~vtkPotreeLoaderThread() {
    std::unique_lock<std::mutex> lock{Mutex};
    Running = false;
    Cond.notify_all();
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

void vtkPotreeLoaderThread::ScheduleForLoading(vtkPotreeMapperNode *node) {
    std::lock_guard<std::mutex> lock{Mutex};
    NeedToLoad.push(node);
    Cond.notify_one();
}

void vtkPotreeLoaderThread::Run() {
    std::unique_lock<std::mutex> lock{mutex_};
    while(Running) {
        while(NeedToLoad.empty()) {
            Cond.wait(lock);
            if(!Running) return;
        }

        vtkPotreeMapperNode* node = NeedToLoad.front();
        NeedToLoad.pop();
        if(node->IsLoaded()) continue; // skip already loaded nodes

        lock.unlock();
        Loader->LoadNodeData(node);
        if(Func) Func();
        lock.lock();
    }
}
