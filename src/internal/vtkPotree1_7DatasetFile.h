//
// Created by mortacious on 12/18/21.
//

#pragma once
#include "vtkPotree1_7DatasetBase.h"

class vtkPotree1_7DatasetFile: public vtkPotree1_7DatasetBase {
public:
    explicit vtkPotree1_7DatasetFile();

    [[nodiscard]] std::unique_ptr<std::istream> FetchFile(const std::string& filename) const override;
};

