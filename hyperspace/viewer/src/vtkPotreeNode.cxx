//
// Created by mortacious on 10/29/21.
//

#include "vtkPotreeNode.h"
#include "vtkPotreeLoader.h"
#include <vtkMapper.h>
#include <vtkWindow.h>
#include <mutex>

vtkPotreeNode::vtkPotreeNode(const std::string& name,
                                   const vtkBoundingBox& bounding_box,
                                   std::weak_ptr<vtkPotreeNode> parent)
        : Name(name), BoundingBox(bounding_box),
          Parent(parent), Loaded(false), PointCount(0)
{
}

vtkPotreeNode::vtkPotreeNode(const std::string &name, const vtkBoundingBox &bounding_box)
: vtkPotreeNode(name, bounding_box, std::weak_ptr<vtkPotreeNode>()){

}

vtkPotreeNode::~vtkPotreeNode() {
}

void vtkPotreeNode::Render(vtkRenderer *ren, vtkActor *a) {
    if(!IsLoaded()) return;
    std::lock_guard<std::mutex> lock{Mutex};
    if(Mapper)
        Mapper->Render(ren, a);
}