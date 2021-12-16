//
// Created by mortacious on 11/3/21.
//

#pragma once
#include <vtkBoundingBox.h>
#include <vtkSmartPointer.h>
#include <vtkWrappingHints.h>
#include <memory>
#include <mutex>
#include <vector>
#include "vtkTileHierarchyModule.h" // For export macro


class vtkMapper;
class vtkTileHierarchyLoader;
class vtkTileHierarchyMapper;
class vtkPolyData;
class vtkRenderer;
class vtkActor;

class vtkTileHierarchyNode;
using vtkTileHierarchyNodePtr = std::shared_ptr<vtkTileHierarchyNode>;

VTK_WRAPEXCLUDE class VTKTILEHIERARCHY_EXPORT vtkTileHierarchyNode
{
public:
    vtkTileHierarchyNode(const std::string& name,
                        const vtkBoundingBox& bounding_box,
                    unsigned int num_children,
                    std::weak_ptr<vtkTileHierarchyNode> parent);

    vtkTileHierarchyNode(const std::string& name,
            const vtkBoundingBox& bounding_box,
                    unsigned int num_children);
    virtual ~vtkTileHierarchyNode();

    const vtkBoundingBox& GetBoundingBox() const {
        return BoundingBox;
    }

    std::size_t GetSize() const {
        return Size;
    }

    const std::weak_ptr<vtkTileHierarchyNode>& GetParent() const {
        return Parent;
    }

    vtkTileHierarchyNodePtr GetChild(vtkIdType idx);

    void SetChild(vtkIdType idx, vtkTileHierarchyNodePtr child);

    unsigned int GetNumChildren() const {
        return Children.size();
    }

    void SetNumChildren(unsigned int num_children);

    bool IsLoaded() const {
        return Mapper != nullptr;
    }

    virtual void Render(vtkRenderer* ren, vtkActor* a);

    const std::string& GetName() const {
        return Name;
    }

    virtual std::size_t GetLevel() const
    {
        return Name.length();
    }

    std::string Name;
    mutable std::mutex Mutex;
    vtkBoundingBox BoundingBox;
    std::weak_ptr<vtkTileHierarchyNode> Parent;
    std::vector<vtkTileHierarchyNodePtr> Children;
    //bool Loaded;
    // This is set dynamically
    std::size_t Size;
    vtkSmartPointer<vtkMapper> Mapper;

protected:
    friend class vtkTileHierarchyLoader;
    friend class vtkTileHierarchyMapper;
private:
    vtkTileHierarchyNode(const vtkTileHierarchyNode&) = delete;
    void operator=(const vtkTileHierarchyNode&) = delete;
};