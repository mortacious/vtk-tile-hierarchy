//
// Created by mortacious on 12/17/21.
//

#pragma once
#include <vtkObject.h>
#include <vtkWrappingHints.h>

#include <vtkSmartPointer.h>
#include <memory>
#include "vtkTileHierarchyLoaderBase.h"
#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyModule.h" // For export macro
#include "gil.h"

class vtkCommand;
class vtkMapper;
class vtkBoundingBox;

class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;

VTK_WRAPEXCLUDE class vtkTileHierarchyLoaderRenderStatePython: public vtkTileHierarchyLoaderRenderState {
public:
    vtkTileHierarchyLoaderRenderStatePython()
            : release() { };
    ~vtkTileHierarchyLoaderRenderStatePython() override {
    };
private:
    gil_scoped_release release;
};

class VTKTILEHIERARCHY_EXPORT vtkPythonLoaderBase: public vtkTileHierarchyLoaderBase {
public:
    static vtkPythonLoaderBase* New();
    vtkTypeMacro(vtkPythonLoaderBase, vtkTileHierarchyLoaderBase);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    VTK_WRAPEXCLUDE void LoadNode(vtkTileHierarchyNodePtr node) override;

    VTK_WRAPEXCLUDE std::unique_ptr<vtkTileHierarchyLoaderRenderState> PreRender() override;

    vtkGetMacro(InitializeEvent, unsigned long);
    vtkGetMacro(ShutdownEvent, unsigned long);
    vtkGetMacro(FetchNodeEvent, unsigned long);


    void Run() override; // make run public here to call it from python

    void SetBoundingBox(const vtkBoundingBox& bbox) {
        BoundingBox = bbox;
    }
protected:
    vtkPythonLoaderBase();
    ~vtkPythonLoaderBase() override = default;

    void DoInitialize() override;
    void DoShutdown() override;

    unsigned long InitializeEvent;
    unsigned long ShutdownEvent;
    unsigned long FetchNodeEvent;

private:
    vtkPythonLoaderBase(const vtkPythonLoaderBase&) = delete;
    void operator=(const vtkPythonLoaderBase&) = delete;

};

