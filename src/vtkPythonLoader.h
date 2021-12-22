//
// Created by mortacious on 12/17/21.
//

#pragma once
#include <vtkObject.h>
#include <vtkWrappingHints.h>

#include <vtkSmartPointer.h>
#include <memory>
#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyModule.h" // For export macro
#include "gil.h"

class vtkCommand;
class vtkMapper;
class vtkBoundingBox;

class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;


class vtkTileHierarchyLoaderRenderStatePython: public vtkTileHierarchyLoaderRenderState {
public:
    vtkTileHierarchyLoaderRenderStatePython()
            : release() { };
    ~vtkTileHierarchyLoaderRenderStatePython() override {
    };
private:
    gil_scoped_release release;
};

class VTKTILEHIERARCHY_EXPORT vtkPythonLoader: public vtkTileHierarchyLoader {
public:
    static vtkPythonLoader* New();
    vtkTypeMacro(vtkPythonLoader, vtkTileHierarchyLoader);

    void PrintSelf(ostream& os, vtkIndent indent) override;
    void Initialize() override;
    void FetchNode(vtkTileHierarchyNodePtr node) override;


    VTK_WRAPEXCLUDE std::unique_ptr<vtkTileHierarchyLoaderRenderState> PreRender() override;

    vtkGetMacro(InitializeEvent, unsigned long);
    vtkGetMacro(FetchNodeEvent, unsigned long);
protected:
    vtkPythonLoader();
    ~vtkPythonLoader() override = default;

    unsigned long InitializeEvent;
    unsigned long FetchNodeEvent;
private:
    vtkPythonLoader(const vtkPythonLoader&) = delete;
    void operator=(const vtkPythonLoader&) = delete;

};

