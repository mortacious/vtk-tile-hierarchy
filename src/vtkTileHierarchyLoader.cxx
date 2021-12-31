//
// Created by mortacious on 11/3/21.
//

#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyNode.h"
#include <vtkBoundingBox.h>
#include <vtkObjectFactory.h>


vtkTileHierarchyLoader::vtkTileHierarchyLoader()
: Threads()
{
    SetNumThreads(1);
}

void vtkTileHierarchyLoader::Shutdown() {
    std::cout << "Shutdown loader" << std::endl;
    UnscheduleAll();
    Superclass::Shutdown();
    for(auto& thread: Threads) {
        if(thread.joinable()) thread.join();
    }
    Stop = false;
}

void vtkTileHierarchyLoader::SetNumThreads(unsigned int num_threads) {
    Shutdown();
    std::cout << "Setting number of threads to " << num_threads << std::endl;
    Threads.reserve(num_threads);
    for(int i=0; i<num_threads; ++i) {
        Threads.emplace_back(&vtkTileHierarchyLoader::Run, this);
    }
}

vtkTileHierarchyLoader::~vtkTileHierarchyLoader() noexcept {
    vtkTileHierarchyLoader::Shutdown();
}

void vtkTileHierarchyLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

void vtkTileHierarchyLoader::Run() {
    while(!Stop) {
        auto node = PopNextNode();
        if(!node) continue;

        LoadNode(node);
        InvokeNodeLoaded();
    }
}

//vtkTileHierarchyNodePtr vtkTileHierarchyLoader::GetRootNode() {
//    if(!RootNode) Initialize();
//    return RootNode;
//}
//
//void vtkTileHierarchyLoader::SetRootNode(vtkTileHierarchyNode* root_node) {
//    Register(root_node);
//    RootNode = vtkTileHierarchyNodePtr(root_node);
//}
//
//vtkSmartPointer<vtkMapper> vtkTileHierarchyLoader::MakeMapper() const {
//    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
//    assert(mapper->GetReferenceCount() == 1);
//    mapper->SetStatic(true);
//    return mapper;
//}
//
//bool vtkTileHierarchyLoader::TryGetNodeFromCache(vtkTileHierarchyNodePtr &node) {
//    std::scoped_lock<std::mutex> cache_lock{CacheMutex};
//    if(Cache.exist(node)) {
//        auto val = Cache.pop(node);
//        std::scoped_lock<std::mutex> node_lock{node->GetMutex()};
//        node->Mapper = vtkSmartPointer(std::move(val.first));
//        node->Size = val.second;
//        return true;
//    }
//    return false;
//}
//
//void vtkTileHierarchyLoader::LoadNode(vtkTileHierarchyNodePtr &node, bool recursive) {
//    std::unique_lock<std::mutex> cache_lock{CacheMutex};
//    if(Cache.exist(node)) {
//        auto val = Cache.pop(node);
//        std::scoped_lock<std::mutex> node_lock{node->GetMutex()};
//        node->Mapper = vtkSmartPointer(std::move(val.first));
//        node->Size = val.second;
//    }
//    cache_lock.unlock();
//    std::unique_lock<std::mutex> node_lock{node->GetMutex()};
//    if(!node->IsLoaded()) {
//        FetchNode(node);
//    }
//    // recursively load all nodes below as well
//    if (recursive)
//    {
//        for (auto& child : node->Children)
//        {
//            if (child)
//                LoadNode(child, true);
//        }
//    }
//}
//
//void vtkTileHierarchyLoader::UnloadNode(vtkTileHierarchyNodePtr &node, bool recursive) {
//    std::unique_lock<std::mutex> node_lock{node->GetMutex()};
//    if(node->IsLoaded() && !Cache.exist(node)) {
//        std::scoped_lock<std::mutex> cache_lock{CacheMutex};
//        // cache this node if it is loaded and not in the cache
//        Cache.put(node, std::make_pair(std::move(node->Mapper), node->GetSize()));
//    }
//    node->Mapper = nullptr; // delete the mapper and it's ressources if not null already
//    node_lock.unlock();
//    if(recursive) {
//        for (auto& child : node->Children) {
//            if(child) {
//                UnloadNode(child, true);
//            }
//        }
//    }
//}
