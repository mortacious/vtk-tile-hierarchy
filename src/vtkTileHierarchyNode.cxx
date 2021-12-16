//
// Created by mortacious on 11/3/21.
//

#include "vtkTileHierarchyNode.h"
#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyMapper.h"
#include <vtkWindow.h>
#include <vtkMapper.h>
#include <mutex>

vtkTileHierarchyNode::vtkTileHierarchyNode(const std::string& name, const vtkBoundingBox& bounding_box,
                                 unsigned int num_children,
                                 std::weak_ptr<vtkTileHierarchyNode> parent)
        : Name(name), BoundingBox(bounding_box),
          Parent(std::move(parent)), Children(num_children, nullptr), Size(0)
{}

vtkTileHierarchyNode::vtkTileHierarchyNode(const std::string& name, const vtkBoundingBox &bounding_box, unsigned int num_children)
        : vtkTileHierarchyNode(name, bounding_box, num_children, std::weak_ptr<vtkTileHierarchyNode>()) {}

vtkTileHierarchyNode::~vtkTileHierarchyNode() {
}

void vtkTileHierarchyNode::Render(vtkRenderer *ren, vtkActor *a) {
    std::lock_guard<std::mutex> lock{Mutex};
    if(!Mapper) return;

    Mapper->Render(ren, a);
}

bool vtkTileHierarchyNode::IsLoaded() const {
    return Mapper;
}

vtkTileHierarchyNodePtr vtkTileHierarchyNode::GetChild(vtkIdType idx) {
    return Children[idx];
}

void vtkTileHierarchyNode::SetNumChildren(unsigned int num_children) {
    Children.resize(num_children, nullptr);
}

void vtkTileHierarchyNode::SetChild(vtkIdType idx, vtkTileHierarchyNodePtr child) {
    if(Children.size() <= idx) {
        SetNumChildren(idx+1);
    }
    auto child_casted = std::dynamic_pointer_cast<vtkTileHierarchyNode>(child);
    Children[idx] = child_casted;
}