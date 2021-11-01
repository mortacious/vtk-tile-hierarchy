//
// Created by mortacious on 10/29/21.
//

#include "vtkPotreeMapperNode.h"
#include "vtkPotreeLoader.h"
#include <vtkMapper.h>
#include <vtkWindow.h>
#include <mutex>

vtkPotreeMapperNode::vtkPotreeMapperNode(const std::string& name,
                                         const vtkBoundingBox& bounding_box,
                                         vtkMapper* mapper_template,
                                         const std::weak_ptr<vtkPotreeMapperNode> parent)
        : Name(name), BoundingBox(bounding_box),
          Parent(parent), Loaded(false), Visible(false),
{
}

vtkPotreeMapperNode::~vtkPotreeMapperNode() {
}

void vtkPotreeMapperNode::Unload(bool recursive) {
    std::lock_guard<std::mutex> lock{Mutex};
    Mapper.Reset();
    Mapper->SetInputDataObject()
    lock.unlock();
    if(recursive) {
        for(const auto& child: Children) {
            if(child) child->SetVisible(visible, true);
        }
    }
}

void vtkPotreeMapperNode::Render(vtkRenderer *ren, vtkActor *a) {
    if(!IsLoaded()) return;
    std::lock_guard<std::mutex> lock{Mutex};
    Mapper->Render()
}

void vtkPotreeMapperNode::SetVisibility(bool visible, bool recursive) {

    std::lock_guard<std::mutex> lock{Mutex};
    Visible = visible;
    lock.unlock();
    if(recursive) {
        for(const auto& child: Children) {
            if(child) child->SetVisible(visible, true);
        }
    }
}

void vtkPotreeMapperNode::LoadData(bool recursive) {
    if(!Loaded) {
        std::string id = Loader->LoadNodeData(this, &Dataset);
        std::lock_guard<std::mutex> lock{Mutex};

        // Create new new Mapper of the same type as the template and copy it's parameters
        Mapper = MapperTemplate->New();
        MapperTemplate->ShallowCopy(&Mapper);
        Mapper->SetInputData(&Dataset);

        Loaded = true;
    }
    if(recursive) {
        for(const auto& child: Children) {
            if(child) child->LoadData(true);
        }
    }
}