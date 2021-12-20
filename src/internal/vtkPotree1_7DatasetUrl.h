//
// Created by mortacious on 12/19/21.
//

#pragma once
#include "vtkPotree1_7DatasetBase.h"
#include <curl/curl.h>

class vtkPotree1_7DatasetUrl: public vtkPotree1_7DatasetBase {
public:
    explicit vtkPotree1_7DatasetUrl();
    ~vtkPotree1_7DatasetUrl();
    [[nodiscard]] std::unique_ptr<std::istream> FetchFile(const std::string& filename) const override;
private:
    CURL* Curl;
};

