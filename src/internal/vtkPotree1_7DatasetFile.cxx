//
// Created by mortacious on 12/18/21.
//

#include "vtkPotree1_7DatasetFile.h"

vtkPotree1_7DatasetFile::vtkPotree1_7DatasetFile()
: vtkPotree1_7DatasetBase() {}

std::string vtkPotree1_7DatasetFile::FetchFile(const std::string &filename) const {
    auto res = std::make_unique<std::ifstream>(filename.c_str());
    if(!res->good())
        throw std::runtime_error(std::string{"Failed to open file: "} + filename);
    auto eos = std::istreambuf_iterator<char>();
    std::string data(std::istreambuf_iterator<char>(*res), eos);
    return std::move(data);
}