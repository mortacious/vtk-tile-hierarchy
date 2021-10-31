//
// Created by mortacious on 10/29/21.
//

#include "vtkPotreeMapperNode.h"
#include <vtkMapper.h>
#include <vtkWindow.h>

vtkPotreeMapperNode::vtkPotreeMapperNode(const std::string& name,
                                         const vtkBoundingBox& bounding_box,
                                         vtkWindow* window,
                                         vtkMapper* mapper,
                                         const std::weak_ptr<vtkPotreeMapperNode> parent)
        : Mapper(mapper), Window(window), Name(name), BoundingBox(bounding_box),
          Parent(parent), Loaded(false), Visible(false),
{
}

vtkPotreeMapperNode::~vtkPotreeMapperNode() {
    Mapper->ReleaseGraphicsResources(Window);
}

void vtkPotreeMapperNode::Unload(bool recursive) {
    std::lock_guard<std::mutex> lock{Mutex};
    Mapper->ReleaseGraphicsResources(Window);
}