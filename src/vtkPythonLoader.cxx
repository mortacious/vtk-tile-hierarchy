//
// Created by mortacious on 12/17/21.
//

#include "vtkPythonLoader.h"
#include "vtkTileHierarchyNode.h"
#include "vtkAttributedTileHierarchyNode.h"
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <vtkCommand.h>
#include <thread>

vtkStandardNewMacro(vtkPythonLoader);

vtkPythonLoader::vtkPythonLoader()
: InitializeEvent(vtkCommand::UserEvent + 1), FetchNodeEvent(vtkCommand::UserEvent + 2) {}

void vtkPythonLoader::Initialize() {
    if(!HasObserver(InitializeEvent)) {
        throw std::runtime_error("Initialization Callback not set.");
    }
    Superclass::InvokeEvent(InitializeEvent);
}

void vtkPythonLoader::FetchNode(vtkTileHierarchyNodePtr node) {
    if(!HasObserver(FetchNodeEvent)) {
        throw std::runtime_error("Fetch Node Callback is not set.");
    }
    Superclass::InvokeEvent(FetchNodeEvent, node);
}

//void vtkPythonLoader::DummyFetch() {
//    vtkBoundingBox bounds(0, 1, 0, 1, 0,1);
//    auto dummy_node = vtkAttributedTileHierarchyNodePtr::New();
//    dummy_node->SetBoundingBox(bounds);
//    dummy_node->SetSize(10);
//    dummy_node->SetAttribute("name", "my_node_name");
//    gil_scoped_release release; // release the Python GIL here so the thread can run!
//    std::thread foo(&vtkPythonLoader::FetchNode, this, dummy_node);
//    foo.join();
//}

void vtkPythonLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}