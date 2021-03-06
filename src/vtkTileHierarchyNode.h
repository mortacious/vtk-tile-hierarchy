//
// Created by mortacious on 11/3/21.
//

#pragma once
#include <vtkBoundingBox.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkWrappingHints.h>
#include <vtkSetGet.h>
#include <vtkObject.h>
#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include "vtkTileHierarchyModule.h" // For export macro


class vtkMapper;
class vtkTileHierarchyLoader;
class vtkTileHierarchyMapper;
class vtkPolyData;
class vtkRenderer;
class vtkActor;

class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = vtkSmartPointer<vtkTileHierarchyNode>;

class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyNode: public vtkObject {
public:
    static vtkTileHierarchyNode* New();
    vtkTypeMacro(vtkTileHierarchyNode, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    const vtkBoundingBox& GetBoundingBox() const {
        return BoundingBox;
    }

    void SetBoundingBox(const vtkBoundingBox& bbox) {
        BoundingBox = bbox;
    }

    ssize_t GetSize() const {
        return Size;
    }

    void SetSize(ssize_t size) {
        Size = size;
    }

    const vtkWeakPointer<vtkTileHierarchyNode>& GetParent() const {
        return Parent;
    }

    void SetParent(vtkTileHierarchyNode* parent) {
        Parent = parent;
    }

    bool HasChild(vtkIdType idx);

    vtkTileHierarchyNode* GetChild(vtkIdType idx) {
        return Children[idx];
    }

    void SetChild(vtkIdType idx, vtkTileHierarchyNode* child);

    unsigned int GetNumChildren() const {
        return Children.size();
    }

    void SetNumChildren(unsigned int num_children);

    bool IsLoaded() const;

    virtual void Render(vtkRenderer* ren, vtkActor* a);

    vtkMapper* GetMapper();

    void SetMapper(vtkMapper* mapper);

    std::mutex& GetMutex() {
        return Mutex;
    }

    void ResetNode();

    void SetLoading();

    bool LoadRequired() const;
protected:
    friend class vtkTileHierarchyLoaderBase;
    friend class vtkTileHierarchyMapper;

    vtkTileHierarchyNode();
    ~vtkTileHierarchyNode() override = default;

    mutable std::mutex Mutex;
    vtkBoundingBox BoundingBox;
    vtkWeakPointer<vtkTileHierarchyNode> Parent;
    std::vector<vtkTileHierarchyNodePtr> Children;

    // This is set dynamically
    ssize_t Size;
    vtkSmartPointer<vtkMapper> Mapper;
    bool Loading;
private:
    vtkTileHierarchyNode(const vtkTileHierarchyNode&) = delete;
    void operator=(const vtkTileHierarchyNode&) = delete;
};