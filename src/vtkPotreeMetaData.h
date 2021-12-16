//
// Created by mortacious on 10/31/21.
//

#pragma once

#include <vtkBoundingBox.h>
#include <filesystem>
#include <string>
#include <vector>
#include <istream>

namespace fs=std::filesystem;

class vtkPotreeLoader;

class vtkPotreeMetaData {
public:
    size_t GetPointCount() const {
        return point_count_;
    }

    void Parse(const std::string path, std::istream& input);
    static size_t SizeOf(const std::string& attr);
private:
    friend class vtkPotreeLoader;

    bool is_file_;
    fs::path octree_dir_;
    fs::path cloud_path_;
    size_t point_count_ = 0;
    size_t hierarchy_step_size_ = 0;
    size_t point_byte_size_;
    vtkBoundingBox bounding_box_;
    float spacing_ = 0;
    float scale_ = 0;
    std::vector<std::string> point_attributes_;
};

