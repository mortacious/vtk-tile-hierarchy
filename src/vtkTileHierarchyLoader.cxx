//
// Created by mortacious on 11/3/21.
//

#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyNode.h"
#include <vtkBoundingBox.h>
#include <vtkObjectFactory.h>


vtkTileHierarchyLoader::vtkTileHierarchyLoader()
: NumThreads(1), Threads()
{
}

void vtkTileHierarchyLoader::Initialize() {
    auto initialized = Initialized;
    Superclass::Initialize();
    if(!initialized) {
        Threads.reserve(NumThreads);
        for(int i=0; i<NumThreads; ++i) {
            Threads.emplace_back(&vtkTileHierarchyLoader::Run, this);
        }
    }

}

void vtkTileHierarchyLoader::Shutdown() {
    auto initialized = Initialized;
    Superclass::Shutdown();
    if(initialized) {
        for (auto &thread: Threads) {
            if (thread.joinable()) thread.join();
        }
        Threads.clear();
    }
}

void vtkTileHierarchyLoader::SetNumThreads(unsigned int num_threads) {
    NumThreads = num_threads;
}

vtkTileHierarchyLoader::~vtkTileHierarchyLoader() noexcept {
    vtkTileHierarchyLoader::Shutdown();
}

void vtkTileHierarchyLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}