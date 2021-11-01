//
// Created by mortacious on 10/31/21.
//

#pragma once

#include "vtkHyperspaceExtensionsModule.h" // For export macro
#include <string>

#include <memory>

class vtkPotreeNode;
using vtkPotreeNodePtr = std::shared_ptr<vtkPotreeNode>;
class vtkPotreeMetaData;
class vtkBoundingBox;

class VTKHYPERSPACEEXTENSIONS_EXPORT vtkPotreeLoader
{
public:
    vtkPotreeLoader(const std::string& path);
    ~vtkPotreeLoader() = default;

    vtkPotreeNodePtr LoadHierarchy();

    void LoadNodeData(vtkPotreeNodePtr& node);

    void UnloadNode(vtkPotreeNodePtr& node);

    static bool IsValidPotree(const std::string& path, std::string& error_msg);
protected:
    friend class vtkPotreeNode;

    void LoadNodeHierarchy(const vtkPotreeNodePtr& root_node) const;

    std::string CreateFileName(const std::string& name, const std::string& extension) const;
    static vtkBoundingBox CreateChildBB(const vtkBoundingBox& parent,
                              int index);
    std::string Path;
    std::unique_ptr<vtkPotreeMetaData> MetaData;
};

