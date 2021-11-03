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
#include <vtkWrappingHints.h>
#include <memory>
#include <mutex>
#include <array>
#include "vtkHyperspaceExtensionsModule.h" // For export macro


class vtkMapper;
class vtkPotreeLoader;
class vtkPolyData;
class vtkRenderer;
class vtkActor;


class vtkPotreeNode;
using vtkPotreeNodePtr = std::shared_ptr<vtkPotreeNode>;

VTK_WRAPEXCLUDE class VTKHYPERSPACEEXTENSIONS_EXPORT vtkPotreeNode
{
public:
    vtkPotreeNode(const std::string& name,
                  const vtkBoundingBox& bounding_box,
                  std::weak_ptr<vtkPotreeNode> parent);
    vtkPotreeNode(const std::string& name,
                  const vtkBoundingBox& bounding_box);
    ~vtkPotreeNode();

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

    std::size_t GetPointCount() const {
        return PointCount;
    }

    const std::weak_ptr<vtkPotreeNode>& GetParent() const {
        return Parent;
    }

    std::array<vtkPotreeNodePtr, 8>& GetChildren() {
        return Children;
    }

    bool IsLoaded() const {
        return Loaded;
    }

    void Render(vtkRenderer* ren, vtkActor* a);
protected:
    friend class vtkPotreeLoader;

    mutable std::mutex Mutex;
    std::string Name;
    vtkBoundingBox BoundingBox;
    std::weak_ptr<vtkPotreeNode> Parent;
    std::array<vtkPotreeNodePtr, 8> Children;
    bool Loaded;

    // This is set dynamically
    std::size_t PointCount;
    vtkSmartPointer<vtkMapper> Mapper;
};

//namespace std {
//
//    template <>
//    struct hash<vtkPotreeNodePtr>
//    {
//        std::size_t operator()(const vtkPotreeNodePtr& k) const
//        {
//            return std::hash<std::string>()(k->GetName());
//        }
//    };
//
//}