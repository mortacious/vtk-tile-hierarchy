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


class vtkPotreeLoader;
class vtkPotreeNode;
using vtkPotreeNodePtr = std::shared_ptr<vtkPotreeNode>;

VTK_WRAPEXCLUDE class VTKHYPERSPACEEXTENSIONS_EXPORT vtkPotreeLoaderThread {
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

    std::function<void()> Func;
    std::mutex Mutex;
    std::condition_variable Cond;
    std::queue<vtkPotreeNodePtr> NeedToLoad;
    std::thread Thread;

};

