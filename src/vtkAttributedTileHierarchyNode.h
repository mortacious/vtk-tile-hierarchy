//
// Created by mortacious on 12/17/21.
//

#pragma once
#include "vtkTileHierarchyNode.h"

class vtkAttributedTileHierarchyNode: vtkTileHierarchyNode {
public:
    vtkAttributedTileHierarchyNode(const std::string& name,
                         const vtkBoundingBox& bounding_box,
                         unsigned int num_children,
                         std::weak_ptr<vtkTileHierarchyNode> parent);

    vtkAttributedTileHierarchyNode(const std::string& name,
                         const vtkBoundingBox& bounding_box,
                         unsigned int num_children);
    virtual ~vtkAttributedTileHierarchyNode();

    const vtkVariant& GetAttribute(const std::string& name);
    void SetAttribute(const std::string& name, const vtkVariant& attribute);
protected:
    std::unordered_map<std::string, vtkVariant> Attributes;

};

