//
// Created by mortacious on 10/31/21.
//

#pragma once

#include "vtkHyperspaceExtensionsModule.h" // For export macro
#include <vtkSmartPointer.h>
#include <vtkWrappingHints.h>
#include <vtkSetGet.h>
#include <vtkStdString.h>

#include <memory>

class vtkPotreeMapperNode;
class vtkPotreeMetaData;

class VTKHYPERSPACEEXTENSIONS_EXPORT vtkPotreeLoader
{
public:
    vtkPotreeLoader(const vtkStdString& path);
    ~vtkPotreeLoader() = default;

    vtkSmartPointer<vtkPotreeMapperNode> LoadHierarchy() const;
    void LoadNodeData(vtkPotreeMapperNode* node, bool recursive = false) const;

    static bool IsValid(const vtkStdString& path, vtkStdString& error_msg);
protected:
    vtkStdString CreateFileName(const vtkStdString& name, const vtkStdString& extension);

    class vtkStdString Path;
    std::unique_ptr<vtkPotreeMetaData> MetaData;
};

