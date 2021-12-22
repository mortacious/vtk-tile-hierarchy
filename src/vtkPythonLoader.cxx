//
// Created by mortacious on 12/17/21.
//

#include "vtkPythonLoader.h"
#include "vtkTileHierarchyNode.h"
#include "vtkAttributedTileHierarchyNode.h"
#include "gil.h"
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

void vtkPythonLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

std::unique_ptr<vtkTileHierarchyLoaderRenderState> vtkPythonLoader::PreRender() {
    return std::make_unique<vtkTileHierarchyLoaderRenderStatePython>();
}