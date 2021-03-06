//
// Created by mortacious on 11/3/21.
//

#pragma once

#include "lruCache.h"
#include <string>
#include <vtkWrappingHints.h>
#include <vtkSmartPointer.h>
#include <vtkObject.h>
#include <thread>
#include <vector>
#include "vtkTileHierarchyLoaderBase.h"
#include "vtkTileHierarchyModule.h" // For export macro

class vtkMapper;

class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyLoader: public vtkTileHierarchyLoaderBase {
public:
    vtkTypeMacro(vtkTileHierarchyLoader, vtkTileHierarchyLoaderBase);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    void SetNumThreads(unsigned int num_threads);

    unsigned int GetNumThreads() const {
        return Threads.size();
    };

    void Shutdown() override;
    void Initialize() override;

protected:
    vtkTileHierarchyLoader();
    ~vtkTileHierarchyLoader() noexcept override;



    unsigned int NumThreads;
    std::vector<std::thread> Threads;
private:
    vtkTileHierarchyLoader(const vtkTileHierarchyLoader&) = delete;
    void operator=(const vtkTileHierarchyLoader&) = delete;
};

