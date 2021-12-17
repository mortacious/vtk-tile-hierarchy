//
// Created by mortacious on 12/17/21.
//

#include "vtkGenericLoader.h"
#include "vtkTileHierarchyNode.h"
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <vtkCommand.h>

vtkStandardNewMacro(vtkTileHierarchyNodeWrapper);

vtkTileHierarchyNodeWrapper::vtkTileHierarchyNodeWrapper() {}

void vtkTileHierarchyNodeWrapper::SetMapper(vtkSmartPointer<vtkMapper> mapper) {
    std::lock_guard<std::mutex> lock{Node->Mutex};

    Node->Mapper = vtkSmartPointer<vtkMapper>(std::move(mapper));
}

vtkMapper * vtkTileHierarchyNodeWrapper::GetMapper() {
    return Node->Mapper;
}

void vtkTileHierarchyNodeWrapper::SetSize(std::size_t size) {
    std::lock_guard<std::mutex> lock{Node->Mutex};
    Node->Size = size;
}

std::size_t vtkTileHierarchyNodeWrapper::GetSize() const {
    return Node->Size;
}

const std::string & vtkTileHierarchyNodeWrapper::GetName() const {
    return Node->Name;
}

const vtkBoundingBox & vtkTileHierarchyNodeWrapper::GetBounds() const {
    return Node->BoundingBox;
}

void vtkTileHierarchyNodeWrapper::ResetNode() {
    std::lock_guard<std::mutex> lock{Node->Mutex};
    Node->Mapper = nullptr;
    Node->Size = 0;
}

const unsigned int vtkTileHierarchyNodeWrapper::GetNumChildren() const {
    return Node->GetNumChildren();
}

void vtkTileHierarchyNodeWrapper::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}


vtkStandardNewMacro(vtkGenericLoader);

vtkGenericLoader::vtkGenericLoader()
: InitializeEvent(vtkCommand::UserEvent + 1), FetchNodeEvent(vtkCommand::UserEvent + 2) {}

void vtkGenericLoader::Initialize() {
    if(!HasObserver(InitializeEvent)) {
        throw std::runtime_error("Initialization Callback not set.");
    }
    Superclass::InvokeEvent(InitializeEvent);
}

void vtkGenericLoader::FetchNode(vtkTileHierarchyNodePtr &node) {
    if(!HasObserver(FetchNodeEvent)) {
        throw std::runtime_error("Fetch Node Callback not set.");
    }
    // only vtkObjects can be passed through the callback
    vtkNew<vtkTileHierarchyNodeWrapper> wrapper;
    wrapper->Node = node;
    Superclass::InvokeEvent(FetchNodeEvent, wrapper);
}

void vtkGenericLoader::DummyFetch() {
    vtkBoundingBox bounds(0, 1, 0, 1, 0,1);
    auto dummy_node = std::make_shared<vtkTileHierarchyNode>("rFoo", bounds, 8);
    FetchNode(dummy_node);
}

void vtkGenericLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}