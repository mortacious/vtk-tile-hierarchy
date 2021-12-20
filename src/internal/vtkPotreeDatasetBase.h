//
// Created by mortacious on 12/19/21.
//

#pragma once
#include <vtkSmartPointer.h>

class vtkPolyData;
class vtkPointHierarchyNode;
using vtkPointHierarchyNodePtr = vtkSmartPointer<vtkPointHierarchyNode>;


class vtkPotreeDatasetBase {
public:
    virtual size_t LoadNode(const vtkPointHierarchyNodePtr& node, vtkSmartPointer<vtkPolyData>& polydata) const = 0;
    virtual void LoadNodeHierarchy(vtkPointHierarchyNodePtr& root_node) const = 0;
    virtual void LoadMetaData(const std::string& path) = 0;

};