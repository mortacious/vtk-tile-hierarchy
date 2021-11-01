//
// Created by mortacious on 10/29/21.
//

/****************************************************************************
 *
 * Adapted from fkie_potree_rviz_plugin:
 *
 * Copyright © 2018 Fraunhofer FKIE
 * Author: Timo Röhling
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#pragma once
#include <vtkBoundingBox.h>
#include <vtkSmartPointer.h>
#include <memory>
#include <mutex>
#include <array>

class vtkMapper;
class vtkPotreeLoader;
class vtkPolyData;
class vtkRenderer;
class vtkActor;

class vtkPotreeMapperNode
{
public:
    vtkPotreeMapperNode(const std::string& name,
                        const vtkBoundingBox& bounding_box,
                        const std::weak_ptr<vtkPotreeMapperNode> parent = std::weak_ptr<vtkPotreeMapperNode>());
    ~vtkPotreeMapperNode();

    const std::string& GetName() const {
        return Name;
    }

    std::size_t GetLevel() const
    {
        return Name.length();
    }

    const vtkBoundingBox& GetBoundingBox() const {
        return BoundingBox;
    }

    const std::weak_ptr<vtkPotreeMapperNode>& GetParent() const {
        return Parent;
    }

    const std::array<std::shared_ptr<vtkPotreeMapperNode>, 8>& GetChildren() const {
        return Children;
    }

    void LoadData(bool recursive = false);

    bool IsLoaded() const {
        return Loaded;
    }

    void Unload(bool recursive = false);

    bool GetVisible() const {
        return Visible;
    }

    void SetVisibility(bool visible, bool recursive = false);

    void Render(vtkRenderer* ren, vtkActor* a);
protected:
    vtkPolyData* Dataset;
    vtkMapper* MapperTemplate;
    vtkNew<vtkMapper> Mapper;
    vtkPotreeLoader* Loader;
private:
    mutable std::mutex Mutex;
    std::string Name;
    vtkBoundingBox BoundingBox;
    std::weak_ptr<vtkPotreeMapperNode> Parent;
    bool Loaded;
    bool Visible;
    std::array<std::shared_ptr<vtkPotreeMapperNode>, 8> Children;
};