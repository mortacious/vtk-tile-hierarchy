//
// Created by mortacious on 11/3/21.
//

#include "vtkTileHierarchyNode.h"
#include "vtkTileHierarchyLoaderBase.h"
#include "vtkTileHierarchyMapper.h"
#include <vtkWindow.h>
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <mutex>

vtkStandardNewMacro(vtkTileHierarchyNode);

vtkTileHierarchyNode::vtkTileHierarchyNode()
: Parent(nullptr), Size(0), Loading(false) {}

void vtkTileHierarchyNode::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

void vtkTileHierarchyNode::Render(vtkRenderer *ren, vtkActor *a) {
    if(!Mapper) return;
    std::scoped_lock<std::mutex> lock{Mutex};
    Mapper->Render(ren, a);
}

bool vtkTileHierarchyNode::IsLoaded() const {
    return Mapper;
}

void vtkTileHierarchyNode::SetLoading() {
    Loading = true;
}

bool vtkTileHierarchyNode::LoadRequired() const {
    return !Loading && !IsLoaded();
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
    Children[idx] = vtkSmartPointer<vtkTileHierarchyNode>(child);
}

void vtkTileHierarchyNode::ResetNode() {
    std::scoped_lock<std::mutex> lock(Mutex);
    Mapper = nullptr;
}

void vtkTileHierarchyNode::SetMapper(vtkMapper* mapper) {
    std::scoped_lock<std::mutex> lock(Mutex);
    Loading = false;
    Mapper = vtkSmartPointer<vtkMapper>(mapper);
}

vtkMapper * vtkTileHierarchyNode::GetMapper() {
    return Mapper;
}