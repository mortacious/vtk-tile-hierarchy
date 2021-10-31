//
// Created by mortacious on 10/31/21.
//

#include "vtkPotreeLoader.h"
#include <filesystem>

namespace fs=std::filesystem;

bool vtkPotreeLoader::IsValid(const vtkStdString& path, vtkStdString& error_msg) {
    fs::path p = {path.c_str()};
    error_msg.clear();
    if(fs::is_directory(p)) {
        error_msg = "not an existing folder";
        return false;
    }
    if(fs::is_regular_file(p / "metadata.json")) {
        error_msg = "unsupported Potree 2.0 format";
        return false;
    }
    if(!fs::is_regular_file(p / "cloud.js")) {
        error_msg = "not a Potree folder";
        return false;
    }
    return true;

    try {
        vtkPotreeMetaData meta_data;
        meta_data.ReadFromJson(p / "cloud.js");
        return true;
    }
}