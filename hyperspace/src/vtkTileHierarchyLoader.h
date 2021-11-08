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
#include "vtkHyperspaceExtensionsModule.h" // For export macro


class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = std::shared_ptr<vtkTileHierarchyNode>;
class vtkMapper;


class VTKHYPERSPACEEXTENSIONS_EXPORT vtkTileHierarchyLoader: public vtkObject {
public:
    vtkTypeMacro(vtkTileHierarchyLoader, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    virtual void Initialize() = 0;

    virtual void LoadNode(vtkTileHierarchyNodePtr& node, bool recursive = false) = 0;

    virtual void UnloadNode(vtkTileHierarchyNodePtr& node, bool recursive = false) = 0;

    virtual vtkMapper* MakeMapper() const;

    bool IsCached(const vtkTileHierarchyNodePtr& node) const;

    void SetMapperTemplate(vtkMapper* mapper);

    vtkMapper* GetMapperTemplate();

    void SetCacheSize(size_t cs) {
        Cache.max_cache_size(cs);
    }

    size_t GetCacheSize() const {
        return Cache.cache_size();
    }

    vtkTileHierarchyNodePtr GetRootNode();
protected:
    vtkTileHierarchyLoader();
    ~vtkTileHierarchyLoader() override = default;

    struct TileTreeNodeSize {
        size_t operator()(const vtkTileHierarchyNodePtr & k , const std::pair<vtkSmartPointer<vtkMapper>, size_t>& v) const;
    };

    vtkSmartPointer<vtkMapper> MapperTemplate;
    vtkTileHierarchyNodePtr RootNode;

    using LRUCacheType = LRUCache<vtkTileHierarchyNodePtr, std::pair<vtkSmartPointer<vtkMapper>, size_t>, TileTreeNodeSize>;
    LRUCacheType Cache;

    void SetRootNode(vtkTileHierarchyNodePtr root_node) {
        RootNode = root_node;
    }
private:
    vtkTileHierarchyLoader(const vtkTileHierarchyLoader&) = delete;
    void operator=(const vtkTileHierarchyLoader&) = delete;
};

