//
// Created by mortacious on 12/18/21.
//

#pragma once
#include "vtkPotreeDatasetBase.h"
#include "vtkPointHierarchyNode.h"
#include <filesystem>
#include <vector>
#include <istream>
#include <string>
#include <vtkBoundingBox.h>

namespace fs=std::filesystem;

class vtkPotree1_7DatasetBase: public vtkPotreeDatasetBase {
public:
    explicit vtkPotree1_7DatasetBase();

    [[nodiscard]] virtual std::unique_ptr<std::istream> FetchFile(const std::string& filename) const = 0;

    static size_t SizeOf(const std::string& attr);

    size_t GetPointCount() const {
        return point_count_;
    }

    size_t LoadNode(const vtkPointHierarchyNodePtr& node, vtkSmartPointer<vtkPolyData>& polydata) const override;
    void LoadNodeHierarchy(vtkPointHierarchyNodePtr& root_node) const override;
    void LoadMetaData(const std::string& path) override;

    bool IsValidPotree(const std::string& path, std::string& error_msg);
protected:
    void LoadNodeHierarchy(vtkPointHierarchyNodePtr& root_node, size_t& points_loaded, size_t& nodes_loaded) const;
    [[nodiscard]] std::string CreateFileName(const std::string& name, const std::string& extension) const;

    static vtkBoundingBox CreateChildBB(const vtkBoundingBox& parent,
                                        int index);

    fs::path octree_dir_;
    fs::path cloud_path_;
    size_t point_count_ = 0;
    size_t hierarchy_step_size_ = 0;
    size_t point_byte_size_{};
    vtkBoundingBox bounding_box_;
    float spacing_ = 0;
    float scale_ = 0;
    std::vector<std::string> point_attributes_;

};

