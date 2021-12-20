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

    std::size_t GetSize() const {
        return Size;
    }

    void SetSize(std::size_t size) {
        Size = size;
    }

    const vtkWeakPointer<vtkTileHierarchyNode>& GetParent() const {
        return Parent;
    }

    void SetParent(vtkTileHierarchyNode* parent) {
        Parent = parent;
    }

    bool HasChild(vtkIdType idx);

    vtkTileHierarchyNodePtr GetChild(vtkIdType idx) {
        return Children[idx];
    }

    void SetChild(vtkIdType idx, vtkTileHierarchyNodePtr child);

    unsigned int GetNumChildren() const {
        return Children.size();
    }

    void SetNumChildren(unsigned int num_children);

    bool IsLoaded() const;

    virtual void Render(vtkRenderer* ren, vtkActor* a);

    vtkMapper* GetMapper();

    void SetMapper(vtkSmartPointer<vtkMapper> mapper);

    std::mutex& GetMutex() {
        return Mutex;
    }

    void ResetNode();
protected:
    friend class vtkTileHierarchyLoader;
    friend class vtkTileHierarchyMapper;

    vtkTileHierarchyNode();
    ~vtkTileHierarchyNode() override = default;

    mutable std::mutex Mutex;
    vtkBoundingBox BoundingBox;
    vtkWeakPointer<vtkTileHierarchyNode> Parent;
    std::vector<vtkTileHierarchyNodePtr> Children;

    // This is set dynamically
    std::size_t Size;
    vtkSmartPointer<vtkMapper> Mapper;
private:
    vtkTileHierarchyNode(const vtkTileHierarchyNode&) = delete;
    void operator=(const vtkTileHierarchyNode&) = delete;
};