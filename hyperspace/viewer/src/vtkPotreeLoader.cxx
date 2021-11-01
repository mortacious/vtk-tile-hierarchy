//
// Created by mortacious on 10/31/21.
//

#include "vtkPotreeLoader.h"
#include "vtkPotreeMetaData.h"
#include "vtkPotreeNode.h"
#include <vtkBoundingBox.h>
#include <vtkVector.h>
#include <filesystem>
#include <queue>
#include <array>
#include <unordered_set>

namespace fs=std::filesystem;


vtkPotreeLoader::vtkPotreeLoader(const std::string &path)
    : Path(path) {
    std::string error_msg;
    if(!IsValidPotree(Path, error_msg)) throw std::runtime_error(error_msg);
    auto cloud_file = fs::path(path.c_str()) / "cloud.js";
    MetaData = std::make_unique<vtkPotreeMetaData>();
    MetaData->ReadFromJson(cloud_file.string());
}

bool vtkPotreeLoader::IsValidPotree(const std::string& path, std::string& error_msg) {
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
}

std::string vtkPotreeLoader::CreateFileName(const std::string &name, const std::string &extension) const {
    fs::path octree_dir = MetaData->cloud_path_ / MetaData->octree_dir_;
    fs::path result;
    std::size_t levels = name.length() / MetaData->hierarchy_step_size_;

    for(std::size_t i = 0; i < levels; ++i) {
        result /= name.substr(i * MetaData->hierarchy_step_size_, MetaData->hierarchy_step_size_);
    }
    result /= std::string("r") + name + extension;
    if(fs::is_regular_file(octree_dir / "u" / result))
        return octree_dir / "u" / result;
    return octree_dir / "r" / result;
}

vtkPotreeNodePtr vtkPotreeLoader::LoadHierarchy() {
    auto root_node = std::make_shared<vtkPotreeNode>("", MetaData->bounding_box_);
    LoadNodeHierarchy(root_node);
    return root_node;
}

void vtkPotreeLoader::LoadNodeHierarchy(const vtkPotreeNodePtr &root_node) const {
    std::queue<vtkPotreeNodePtr> pending_nodes;
    pending_nodes.push(root_node);

    char cfg[5];
    fs::path hrc_file = CreateFileName(root_node->GetName(), ".hrc");
    std::ifstream f{hrc_file.c_str()};
    if(!f.good())
        throw std::runtime_error(std::string{"Failed to read file: "} + hrc_file.string());
    f.read(cfg, 5);
    while(f.good())
    {
        auto node = pending_nodes.front();
        pending_nodes.pop();
        for(int j=0; j<8; ++j) {
            if(cfg[0] & (1 << j)) {
                if(!node->GetChildren()[j]) {
                    auto child = std::make_shared<vtkPotreeNode>(node->GetName() + std::to_string(j), CreateChildBB(node->GetBoundingBox(), j), node);
                    node->Children[j] = child;
                }
                pending_nodes.push(node->Children[j]);
            }
        }
        f.read(cfg, 5); // read next node
    }

    std::unordered_set<vtkPotreeNode*> seen;    // save the shared_ptr copy overhead and just
                                                // track seen nodes by their address

    while(!pending_nodes.empty()) {
        auto node = pending_nodes.front()->Parent.lock();
        pending_nodes.pop();
        if(node && seen.insert(node.get()).second)
            LoadNodeHierarchy(node);
    }
}

vtkBoundingBox vtkPotreeLoader::CreateChildBB(const vtkBoundingBox &parent, int index) {
    vtkVector3d min{parent.GetMinPoint()};
    vtkVector3d max{parent.GetMaxPoint()};
    vtkVector3d half_size;
    parent.GetLengths(half_size.GetData());
    half_size[0] /= 2;
    half_size[1] /= 2;
    half_size[2] /= 2;

    if(index & 1)
        min[2] += half_size[2];
    else
        max[2] -= half_size[2];

    if(index & 2)
        min[1] += half_size[1];
    else
        min[1] -= half_size[1];

    if(index & 4)
        min[0] += half_size[0];
    else
        max[0] -= half_size[0];

    return vtkBoundingBox(min[0], max[0], min[1], max[1], min[2], max[2]);
}

void vtkPotreeLoader::LoadNodeData(vtkPotreeNodePtr &node) {

}

void vtkPotreeLoader::UnloadNode(vtkPotreeNodePtr &node) {

}
