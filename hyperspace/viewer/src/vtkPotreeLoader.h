//
// Created by mortacious on 10/31/21.
//

#pragma once

#include "vtkHyperspaceExtensionsModule.h" // For export macro
#include <vtkSmartPointer.h>
#include <vtkWrappingHints.h>
#include <vtkDataSet.h>
#include <vtkSetGet.h>
#include <string>

#include <memory>

class vtkPotreeMapperNode;
class vtkPotreeMetaData;

class VTKHYPERSPACEEXTENSIONS_EXPORT vtkPotreeLoader
{
public:
    vtkPotreeLoader(const std::string& path);
    ~vtkPotreeLoader() = default;

    vtkSmartPointer<vtkPotreeMapperNode> CreateRootNode() const;

    static bool IsValid(const std::string& path, std::string& error_msg);
protected:
    friend class vtkPotreeMapperNode;

    void LoadNodeData(const vtkPotreeMapperNode& node, vtkDataSet* dataset) const;

    vtkStdString CreateFileName(const std::string& name, const std::string& extension);

    std::string Path;
    std::unique_ptr<vtkPotreeMetaData> MetaData;
};

