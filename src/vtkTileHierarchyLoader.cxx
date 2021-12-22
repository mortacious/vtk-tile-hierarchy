//
// Created by mortacious on 11/3/21.
//

#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyNode.h"
#include "vtkTileHierarchyLoaderThread.h"
#include <vtkBoundingBox.h>
#include <vtkObjectFactory.h>
#include <vtkMapper.h>
#include <vtkDataSetMapper.h>


std::size_t std::hash<vtkTileHierarchyNodePtr>::operator()(const vtkTileHierarchyNodePtr &s) const noexcept {
    return std::hash<vtkTileHierarchyNode *>{}(s.GetPointer());
}

size_t vtkTileHierarchyLoader::TileTreeNodeSize::operator()(const vtkTileHierarchyNodePtr &k, const std::pair<vtkSmartPointer<vtkMapper>, size_t> &v) const {
    if(!k) return 0;
    return k->GetSize();
}

vtkTileHierarchyLoader::vtkTileHierarchyLoader()
    : RootNode(nullptr), Cache(15000000) {

}

void vtkTileHierarchyLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

vtkTileHierarchyNodePtr vtkTileHierarchyLoader::GetRootNode() {
    if(!RootNode) Initialize();
    return RootNode;
}

void vtkTileHierarchyLoader::SetRootNode(vtkTileHierarchyNode* root_node) {
    Register(root_node);
    RootNode = vtkTileHierarchyNodePtr(root_node);
}

vtkSmartPointer<vtkMapper> vtkTileHierarchyLoader::MakeMapper() const {
    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    assert(mapper->GetReferenceCount() == 1);
    mapper->SetStatic(true);
    return mapper;
}

bool vtkTileHierarchyLoader::TryGetNodeFromCache(vtkTileHierarchyNodePtr &node) {
    std::scoped_lock<std::mutex> cache_lock{CacheMutex};
    if(Cache.exist(node)) {
        auto val = Cache.pop(node);
        std::scoped_lock<std::mutex> node_lock{node->GetMutex()};
        node->Mapper = vtkSmartPointer(std::move(val.first));
        node->Size = val.second;
        return true;
    }
    return false;
}

void vtkTileHierarchyLoader::LoadNode(vtkTileHierarchyNodePtr &node, bool recursive) {
    std::unique_lock<std::mutex> cache_lock{CacheMutex};
    if(Cache.exist(node)) {
        auto val = Cache.pop(node);
        std::scoped_lock<std::mutex> node_lock{node->GetMutex()};
        node->Mapper = vtkSmartPointer(std::move(val.first));
        node->Size = val.second;
    }
    cache_lock.unlock();
    std::unique_lock<std::mutex> node_lock{node->GetMutex()};
    if(!node->IsLoaded()) {
        FetchNode(node);
    }
    // recursively load all nodes below as well
    if (recursive)
    {
        for (auto& child : node->Children)
        {
            if (child)
                LoadNode(child, true);
        }
    }
}

void vtkTileHierarchyLoader::UnloadNode(vtkTileHierarchyNodePtr &node, bool recursive) {
    std::unique_lock<std::mutex> node_lock{node->GetMutex()};
    if(node->IsLoaded() && !Cache.exist(node)) {
        std::scoped_lock<std::mutex> cache_lock{CacheMutex};
        // cache this node if it is loaded and not in the cache
        Cache.put(node, std::make_pair(std::move(node->Mapper), node->GetSize()));
    }
    node->Mapper = nullptr; // delete the mapper and it's ressources if not null already
    node_lock.unlock();
    if(recursive) {
        for (auto& child : node->Children) {
            if(child) {
                UnloadNode(child, true);
            }
        }
    }
}
