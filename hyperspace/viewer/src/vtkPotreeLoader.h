//
// Created by mortacious on 10/31/21.
//

#pragma once

#include "vtkHyperspaceExtensionsModule.h" // For export macro
#include "lruCache.h"
#include <string>
#include <vtkSmartPointer.h>
#include <vtkObject.h>
#include <memory>

class vtkPotreeNode;
using vtkPotreeNodePtr = std::shared_ptr<vtkPotreeNode>;
class vtkPotreeMetaData;
class vtkBoundingBox;
class vtkMapper;

struct PotreeNodeSize {
    size_t operator()(const vtkPotreeNodePtr & k , const std::pair<vtkSmartPointer<vtkMapper>, size_t>& v) const;
};

class VTKHYPERSPACEEXTENSIONS_EXPORT vtkPotreeLoader: public vtkObject
{
public:
    static vtkPotreeLoader* New();
    vtkTypeMacro(vtkPotreeLoader, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    void LoadMetaData();
    vtkPotreeNodePtr LoadHierarchy();

    void LoadNode(vtkPotreeNodePtr& node, bool recursive = false);

    void UnloadNode(vtkPotreeNodePtr& node, bool recursive = false);

    void SetPath(const std::string& path) {
        Path = path;
    }

    const std::string& GetPath() const {
        return Path;
    }

    void SetTemplateMapper(vtkMapper* mapper) {
        MapperTemplate.TakeReference(mapper);
    }

    vtkMapper* GetTemplateMapper() const {
        return MapperTemplate;
    }

    void SetCacheSize(size_t cs) {
        Cache.cache_size(cs);
    }

    size_t GetCacheSize() const {
        return Cache.cache_size();
    }

    static bool IsValidPotree(const std::string& path, std::string& error_msg);
protected:
    vtkPotreeLoader();
    ~vtkPotreeLoader() = default;

    friend class vtkPotreeNode;

    void LoadNodeHierarchy(const vtkPotreeNodePtr& root_node) const;
    void LoadNodeFromFile(vtkPotreeNodePtr& node);

    std::string CreateFileName(const std::string& name, const std::string& extension) const;
    static vtkBoundingBox CreateChildBB(const vtkBoundingBox& parent,
                              int index);
    std::string Path;
    vtkSmartPointer<vtkMapper> MapperTemplate;
    std::unique_ptr<vtkPotreeMetaData> MetaData;
    using LRUCacheType = LRUCache<vtkPotreeNodePtr, std::pair<vtkSmartPointer<vtkMapper>, size_t>, PotreeNodeSize>;
    LRUCacheType Cache;
private:
    vtkPotreeLoader(const vtkPotreeLoader&) = delete;
    void operator=(const vtkPotreeLoader&) = delete;
};

