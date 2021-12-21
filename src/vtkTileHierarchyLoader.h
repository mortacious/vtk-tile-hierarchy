//
// Created by mortacious on 11/3/21.
//

#pragma once

#include "lruCache.h"
#include <string>
#include <vtkWrappingHints.h>
#include <vtkSmartPointer.h>
#include <vtkObject.h>
#include <memory>
#include "vtkTileHierarchyModule.h" // For export macro


class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;

// custom specialization of std::hash for node pointers
template<>
struct std::hash<vtkTileHierarchyNodePtr> {
    std::size_t operator()(vtkTileHierarchyNodePtr const &s) const noexcept;
};

class vtkMapper;

class vtkTileHierarchyLoaderThread;

class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyLoader: public vtkObject {
public:
    vtkTypeMacro(vtkTileHierarchyLoader, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    virtual void Initialize() = 0;

    VTK_WRAPEXCLUDE void LoadNode(vtkTileHierarchyNodePtr& node) {
        LoadNode(node, false);
    }

    VTK_WRAPEXCLUDE virtual void LoadNode(vtkTileHierarchyNodePtr& node, bool recursive);

    VTK_WRAPEXCLUDE void UnloadNode(vtkTileHierarchyNodePtr& node) {
        UnloadNode(node, false);
    }

    VTK_WRAPEXCLUDE void UnloadNode(vtkTileHierarchyNodePtr& node, bool recursive);

    VTK_WRAPEXCLUDE virtual void FetchNode(vtkTileHierarchyNodePtr node) = 0;

    VTK_WRAPEXCLUDE bool TryGetNodeFromCache(vtkTileHierarchyNodePtr& node);

    virtual vtkSmartPointer<vtkMapper> MakeMapper() const;

    //bool IsCached(const vtkTileHierarchyNodePtr& node) const;

    //void SetMapperTemplate(vtkMapper* mapper);

    //vtkMapper* GetMapperTemplate();

    void SetCacheSize(size_t cs) {
        Cache.max_cache_size(cs);
    }

    size_t GetCacheSize() const {
        return Cache.cache_size();
    }

    vtkTileHierarchyNodePtr GetRootNode();
    void SetRootNode(vtkTileHierarchyNode* root_node);
protected:
    friend class vtkTileHierarchyLoaderThread;
    vtkTileHierarchyLoader();
    ~vtkTileHierarchyLoader() override = default;

    struct TileTreeNodeSize {
        size_t operator()(const vtkTileHierarchyNodePtr & k , const std::pair<vtkSmartPointer<vtkMapper>, size_t>& v) const;
    };

    vtkTileHierarchyNodePtr RootNode;

    using LRUCacheType = LRUCache<vtkTileHierarchyNodePtr, std::pair<vtkSmartPointer<vtkMapper>, size_t>, TileTreeNodeSize>;
    LRUCacheType Cache;

    std::mutex CacheMutex;

private:
    vtkTileHierarchyLoader(const vtkTileHierarchyLoader&) = delete;
    void operator=(const vtkTileHierarchyLoader&) = delete;
};

