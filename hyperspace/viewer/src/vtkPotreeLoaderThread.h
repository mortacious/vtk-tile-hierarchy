//
// Created by mortacious on 10/31/21.
//

#pragma once
#include <vtkSmartPointer.h>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <queue>

class vtkPotreeLoader;
class vtkPotreeNode;
using vtkPotreeNodePtr = std::shared_ptr<vtkPotreeNode>;

class vtkPotreeLoaderThread {
public:
    explicit vtkPotreeLoaderThread(vtkPotreeLoader* loader);
    ~vtkPotreeLoaderThread();
    void UnscheduleAll();
    void ScheduleForLoading(vtkPotreeNodePtr& node);
    void SetNodeLoadedCallBack(const std::function<void()>& func);
private:
    void Run();

    vtkSmartPointer<vtkPotreeLoader> Loader;
    bool Running;
    std::thread Thread;

    std::function<void()> Func;
    std::mutex Mutex;
    std::condition_variable Cond;
    std::queue<vtkPotreeNodePtr> NeedToLoad;
};

