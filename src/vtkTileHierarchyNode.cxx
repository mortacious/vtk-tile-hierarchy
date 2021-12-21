//
// Created by mortacious on 11/3/21.
//

#include "vtkTileHierarchyNode.h"
#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyMapper.h"
#include <vtkWindow.h>
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <mutex>

vtkStandardNewMacro(vtkTileHierarchyNode);

vtkTileHierarchyNode::vtkTileHierarchyNode()
: Parent(nullptr), Size(0) {}

void vtkTileHierarchyNode::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

void vtkTileHierarchyNode::Render(vtkRenderer *ren, vtkActor *a) {
    std::lock_guard<std::mutex> lock{Mutex};
    if(!Mapper) return;

    Mapper->Render(ren, a);
}

bool vtkTileHierarchyNode::IsLoaded() const {
    return Mapper;
}

void vtkTileHierarchyNode::SetNumChildren(unsigned int num_children) {
    Children.resize(num_children, nullptr);
}

bool vtkTileHierarchyNode::HasChild(vtkIdType idx) {
    if(Children.size() <= idx) {
        return false;
    }
    return Children[idx] != nullptr;
}

void vtkTileHierarchyNode::SetChild(vtkIdType idx, vtkTileHierarchyNode* child) {
    if(Children.size() <= idx) {
        SetNumChildren(idx+1);
    }
    Register(child);
    Children[idx].TakeReference(child);
}

void vtkTileHierarchyNode::ResetNode() {
    Mapper = nullptr;
    Size = 0;
}

void vtkTileHierarchyNode::SetMapper(vtkSmartPointer<vtkMapper> mapper) {
    Mapper = vtkSmartPointer<vtkMapper>(std::move(mapper));
}

vtkMapper * vtkTileHierarchyNode::GetMapper() {
    return Mapper;
}