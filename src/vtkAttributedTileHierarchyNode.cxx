//
// Created by mortacious on 12/17/21.
//

#include "vtkAttributedTileHierarchyNode.h"
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkAttributedTileHierarchyNode);

vtkAttributedTileHierarchyNode::vtkAttributedTileHierarchyNode() {}


void vtkAttributedTileHierarchyNode::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

const vtkVariant& vtkAttributedTileHierarchyNode::GetAttribute(const std::string &name) {
    auto attr = Attributes.find(name);
    if(attr == Attributes.end()) {
        throw std::runtime_error("Attribute not found");
    }
    return attr->second;
}

void vtkAttributedTileHierarchyNode::SetAttribute(const std::string &name, const vtkVariant &attribute) {
    Attributes.emplace(name, attribute);
}