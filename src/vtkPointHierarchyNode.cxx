//
// Created by mortacious on 10/29/21.
//

#include "vtkPointHierarchyNode.h"
#include "vtkPotreeLoader.h"
#include <vtkPointGaussianMapper.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <mutex>

vtkPointHierarchyNode::vtkPointHierarchyNode(const std::string& name,
                             const vtkBoundingBox& bounding_box,
                             std::weak_ptr<vtkPointHierarchyNode> parent)
        : vtkTileHierarchyNode(name, bounding_box, 8, parent), Scale(true)
{
}

vtkPointHierarchyNode::vtkPointHierarchyNode(const std::string &name, const vtkBoundingBox &bounding_box)
: vtkPointHierarchyNode(name, bounding_box, std::weak_ptr<vtkPointHierarchyNode>()){

}

void vtkPointHierarchyNode::Render(vtkRenderer *ren, vtkActor *a) {
    if(!IsLoaded()) return;
    a->GetProperty()->SetRepresentationToPoints();
    a->GetProperty()->SetPointSize(3);
    vtkTileHierarchyNode::Render(ren, a); // Do the actual rendering
}