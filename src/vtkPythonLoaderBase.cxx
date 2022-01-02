//
// Created by mortacious on 12/17/21.
//

#include "vtkPythonLoaderBase.h"
#include "vtkTileHierarchyNode.h"
#include "gil.h"
#include <vtkMapper.h>
#include <vtkObjectFactory.h>
#include <vtkCommand.h>
#include <thread>

vtkStandardNewMacro(vtkPythonLoaderBase);

vtkPythonLoaderBase::vtkPythonLoaderBase()
: InitializeEvent(vtkCommand::UserEvent + 1),
  ShutdownEvent(vtkCommand::UserEvent + 2),
  FetchNodeEvent(vtkCommand::UserEvent + 3) {}

void vtkPythonLoaderBase::DoInitialize() {
    if(!HasObserver(InitializeEvent)) {
        throw std::runtime_error("InitializeEvent callback not set.");
    }
    Superclass::InvokeEvent(InitializeEvent); // call into python
}

void vtkPythonLoaderBase::DoShutdown() {
    if(!HasObserver(ShutdownEvent)) {
        throw std::runtime_error("ShutdownEvent callback not set.");
    }
    Superclass::InvokeEvent(ShutdownEvent); // call into python
}

void vtkPythonLoaderBase::LoadNode(vtkTileHierarchyNodePtr node) {
    if(!HasObserver(FetchNodeEvent)) {
        throw std::runtime_error("Fetch Node Callback is not set.");
    }
    Superclass::InvokeEvent(FetchNodeEvent, node); // call into python
}

void vtkPythonLoaderBase::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

std::unique_ptr<vtkTileHierarchyLoaderRenderState> vtkPythonLoaderBase::PreRender() {
    return std::make_unique<vtkTileHierarchyLoaderRenderStatePython>();
}

void vtkPythonLoaderBase::Run() {
    gil_scoped_release release; // release the gil so other python threads can run
    Superclass::Run();
}

