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

class vtkCommand;
class vtkMapper;
class vtkBoundingBox;

class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = std::shared_ptr<vtkTileHierarchyNode>;

class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyNodeWrapper: public vtkObject {
public:
    static vtkTileHierarchyNodeWrapper* New();
    vtkTypeMacro(vtkTileHierarchyNodeWrapper, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    void SetMapper(vtkSmartPointer<vtkMapper> mapper);
    vtkMapper* GetMapper();

    std::size_t GetSize() const;
    void SetSize(std::size_t size);

    const std::string& GetName() const;
    const vtkBoundingBox& GetBounds() const;

    const unsigned int GetNumChildren() const;


    void ResetNode();
protected:
    friend class vtkGenericLoader;

    vtkTileHierarchyNodeWrapper();
    ~vtkTileHierarchyNodeWrapper() override = default;


    vtkTileHierarchyNodePtr Node;
private:
    vtkTileHierarchyNodeWrapper(const vtkTileHierarchyNodeWrapper&) = delete;
    void operator=(const vtkTileHierarchyNodeWrapper&) = delete;
};

class VTKTILEHIERARCHY_EXPORT vtkGenericLoader: public vtkTileHierarchyLoader {
public:
    static vtkGenericLoader* New();
    vtkTypeMacro(vtkGenericLoader, vtkObject);

    void PrintSelf(ostream& os, vtkIndent indent) override;
    void Initialize() override;
    void FetchNode(vtkTileHierarchyNodePtr& node) override;

    void DummyFetch();

    vtkGetMacro(InitializeEvent, unsigned long);
    vtkGetMacro(FetchNodeEvent, unsigned long);
protected:
    vtkGenericLoader();
    ~vtkGenericLoader() override = default;

    unsigned long InitializeEvent;
    unsigned long FetchNodeEvent;
private:
    vtkGenericLoader(const vtkGenericLoader&) = delete;
    void operator=(const vtkGenericLoader&) = delete;

};

