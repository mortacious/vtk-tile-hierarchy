//
// Created by mortacious on 10/31/21.
//

#pragma once

#include "vtkTileHierarchyModule.h" // For export macro
#include "vtkTileHierarchyLoader.h"
#include "lruCache.h"
#include <string>
#include <vtkWrappingHints.h>
#include <vtkSmartPointer.h>
#include <vtkObject.h>
#include <memory>

class vtkPointHierarchyNode;
using vtkPointHierarchyNodePtr = vtkSmartPointer<vtkPointHierarchyNode>;
class vtkPotreeDatasetBase;
class vtkBoundingBox;
class vtkMapper;

class VTKTILEHIERARCHY_EXPORT vtkPotreeLoader: public vtkTileHierarchyLoader
{
public:
    static vtkPotreeLoader* New();
    vtkTypeMacro(vtkPotreeLoader, vtkTileHierarchyLoader);
    void PrintSelf(ostream& os, vtkIndent indent) override;


    VTK_WRAPEXCLUDE void LoadNode(vtkTileHierarchyNodePtr node) override;


    vtkSmartPointer<vtkMapper> MakeMapper() const;

    void SetPath(const std::string& path) {
        Path = path;
    }

    const std::string& GetPath() const {
        return Path;
    }
protected:
    vtkPotreeLoader();
    ~vtkPotreeLoader() override = default;

    void DoInitialize() override;

    std::string Path;
    std::unique_ptr<vtkPotreeDatasetBase> Dataset;
private:
    vtkPotreeLoader(const vtkPotreeLoader&) = delete;
    void operator=(const vtkPotreeLoader&) = delete;
};

