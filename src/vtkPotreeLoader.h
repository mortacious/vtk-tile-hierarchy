//
// Created by mortacious on 10/31/21.
//

#pragma once

#include "vtkTileHierarchyModule.h" // For export macro
#include "vtkTileHierarchyLoader.h"
#include "lruCache.h"
#include <string>
#include <vtkWrappingHints.h>
#include <vtkSmartPointer.h>
#include <vtkObject.h>
#include <memory>

class vtkPointHierarchyNode;
using vtkPointHierarchyNodePtr = std::shared_ptr<vtkPointHierarchyNode>;
class vtkPotreeMetaData;
class vtkBoundingBox;
class vtkMapper;

class VTKTILEHIERARCHY_EXPORT vtkPotreeLoader: public vtkTileHierarchyLoader
{
public:
    static vtkPotreeLoader* New();
    vtkTypeMacro(vtkPotreeLoader, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    virtual void Initialize();

    //VTK_WRAPEXCLUDE void LoadNode(vtkTileHierarchyNodePtr& node, bool recursive = false) override;

    VTK_WRAPEXCLUDE void FetchNode(vtkTileHierarchyNodePtr& node) override;


    vtkSmartPointer<vtkMapper> MakeMapper() const override;

    void SetPath(const std::string& path) {
        Path = path;
    }

    const std::string& GetPath() const {
        return Path;
    }

    static bool IsValidPotree(const std::string& path, std::string& error_msg);
protected:
    enum class DatasetLoc {
        UNDEFINED = 0,
        FILESYSTEM,
        REMOTE
    };

    vtkPotreeLoader();
    ~vtkPotreeLoader() = default;

    //using vtkTileHierarchyLoader::SetMapperTemplate;

    vtkMapper* GetTemplateMapper();

    void LoadMetaData();
    void LoadNodeHierarchy(const vtkPointHierarchyNodePtr& root_node, size_t& points_loaded) const;
    void LoadNodeFromFile(vtkPointHierarchyNodePtr& node);

    std::unique_ptr<std::istream> FetchFile(const std::string& filename) const;

    std::string CreateFileName(const std::string& name, const std::string& extension) const;
    static vtkBoundingBox CreateChildBB(const vtkBoundingBox& parent,
                              int index);

    std::string Path;
    DatasetLoc Location;
    std::unique_ptr<vtkPotreeMetaData> MetaData;
private:
    vtkPotreeLoader(const vtkPotreeLoader&) = delete;
    void operator=(const vtkPotreeLoader&) = delete;
};

