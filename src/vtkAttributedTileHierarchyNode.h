//
// Created by mortacious on 12/17/21.
//

#pragma once
#include "vtkTileHierarchyNode.h"
#include <unordered_map>
#include <vtkVariant.h>

class vtkVariant;

class vtkAttributedTileHierarchyNode;
using vtkAttributedTileHierarchyNodePtr = vtkSmartPointer<vtkAttributedTileHierarchyNode>;

class vtkAttributedTileHierarchyNode: public vtkTileHierarchyNode {
public:
    static vtkAttributedTileHierarchyNode* New();
    vtkTypeMacro(vtkAttributedTileHierarchyNode, vtkTileHierarchyNode);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    const vtkVariant& GetAttribute(const std::string& name);
    void SetAttribute(const std::string& name, const vtkVariant& attribute);
protected:
    vtkAttributedTileHierarchyNode();
    ~vtkAttributedTileHierarchyNode() override = default;

    std::unordered_map<std::string, vtkVariant> Attributes;
private:
    vtkAttributedTileHierarchyNode(const vtkAttributedTileHierarchyNode&) = delete;
    void operator=(const vtkAttributedTileHierarchyNode&) = delete;
};

