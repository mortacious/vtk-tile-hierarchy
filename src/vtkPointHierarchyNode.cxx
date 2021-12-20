//
// Created by mortacious on 10/29/21.
//

#include "vtkPointHierarchyNode.h"
#include "vtkPotreeLoader.h"
#include <vtkPointGaussianMapper.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <mutex>

vtkStandardNewMacro(vtkPointHierarchyNode);

vtkPointHierarchyNode::vtkPointHierarchyNode()
: Scale(true) {}


void vtkPointHierarchyNode::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}
//
//void vtkPointHierarchyNode::Render(vtkRenderer *ren, vtkActor *a) {
//    if(!IsLoaded()) return;
//    a->GetProperty()->SetRepresentationToPoints();
//    a->GetProperty()->SetPointSize(3);
//    vtkTileHierarchyNode::Render(ren, a); // Do the actual rendering
//}