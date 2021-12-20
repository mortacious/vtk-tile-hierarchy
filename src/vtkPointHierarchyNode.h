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
#include "vtkTileHierarchyNode.h"
#include "vtkTileHierarchyModule.h" // For export macro



class vtkMapper;
class vtkPotreeLoader;
class vtkPolyData;
class vtkRenderer;
class vtkActor;


class vtkPointHierarchyNode;
using vtkPointHierarchyNodePtr = vtkSmartPointer<vtkPointHierarchyNode>;

class VTKTILEHIERARCHY_EXPORT vtkPointHierarchyNode: public vtkTileHierarchyNode
{
public:
    static vtkPointHierarchyNode* New();
    vtkTypeMacro(vtkPointHierarchyNode, vtkTileHierarchyNode);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    //void Render(vtkRenderer* ren, vtkActor* a) override;

    const std::string& GetName() const {
        return Name;
    }

    void SetName(const std::string& name) {
        Name = name;
    }

    unsigned int GetLevel() const {
        return Name.length();

    }

    vtkGetMacro(Scale, bool);
    vtkSetMacro(Scale, bool);
protected:
    vtkPointHierarchyNode();
    ~vtkPointHierarchyNode() override = default;

    std::string Name;
    bool Scale = true;
};