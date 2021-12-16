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


size_t vtkTileHierarchyLoader::TileTreeNodeSize::operator()(const vtkTileHierarchyNodePtr &k, const std::pair<vtkSmartPointer<vtkMapper>, size_t> &v) const {
    if(!k) return 0;
    return k->GetSize();
}

vtkTileHierarchyLoader::vtkTileHierarchyLoader()
    : MapperTemplate(vtkDataSetMapper::New()), RootNode(nullptr), Cache(15000000) {
    MapperTemplate->SetColorModeToMapScalars();
    MapperTemplate->SetScalarModeToUsePointData();
    MapperTemplate->SetStatic(true);

}

void vtkTileHierarchyLoader::SetMapperTemplate(vtkMapper *mapper) {
    MapperTemplate.TakeReference(mapper);
}

vtkMapper * vtkTileHierarchyLoader::GetMapperTemplate() {
    return MapperTemplate.Get();
}

void vtkTileHierarchyLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

bool vtkTileHierarchyLoader::IsCached(const vtkTileHierarchyNodePtr &node) const {
    return Cache.exist(node);
}

vtkTileHierarchyNodePtr vtkTileHierarchyLoader::GetRootNode() {
    if(!RootNode) Initialize();
    return RootNode;
}

vtkSmartPointer<vtkMapper> vtkTileHierarchyLoader::MakeMapper() const {
    //auto mapper = vtkSmartPointer<vtkMapper>::NewInstance(MapperTemplate);
    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    assert(mapper->GetReferenceCount() == 1);
    mapper->SetStatic(true);
    return mapper;
}

bool vtkTileHierarchyLoader::TryGetNodeFromCache(vtkTileHierarchyNodePtr &node) {
    std::unique_lock<std::mutex> node_lock{node->Mutex};
    if(Cache.exist(node)) {
        std::scoped_lock<std::mutex> cache_lock{CacheMutex};
        auto val = Cache.pop(node);
        node->Mapper = vtkSmartPointer(std::move(val.first));
        node->Size = val.second;
        //Cache.erase(node); // Remove the node from cache as long as it is used by the mapper
        node_lock.unlock();
        return true;
    }
    if(node_lock) node_lock.unlock();
    return false;
}

void vtkTileHierarchyLoader::LoadNode(vtkTileHierarchyNodePtr &node, bool recursive) {
    if(!TryGetNodeFromCache(node) && !node->IsLoaded()) { // avoid loading nodes that are already in use or cached
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
    std::unique_lock<std::mutex> node_lock{node->Mutex};
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
