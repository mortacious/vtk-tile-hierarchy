//
// Created by mortacious on 10/31/21.
//

#include "vtkPotreeLoader.h"
#include "vtkPotreeMetaData.h"
#include "vtkPointHierarchyNode.h"
#include <vtkBoundingBox.h>
#include <vtkVector.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkMapper.h>
#include <vtkPointGaussianMapper.h>
#include <vtkIdTypeArray.h>
#include <filesystem>
#include <queue>
#include <array>
#include <unordered_set>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkScalarsToColors.h>

namespace fs=std::filesystem;

vtkStandardNewMacro(vtkPotreeLoader);

vtkPotreeLoader::vtkPotreeLoader()
    : Path()
{
    MapperTemplate = vtkSmartPointer<vtkPointGaussianMapper>::New();
}

bool vtkPotreeLoader::IsValidPotree(const std::string& path, std::string& error_msg) {
    fs::path p = {path.c_str()};
    error_msg.clear();
    if(!fs::is_directory(p)) {
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

std::istream vtkPotreeLoader::FetchFile(const std::string &filename) const {
    if(filename.rfind("http://", 0)) {
        // this is an url so fetch the file from the remote location
    }
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

void vtkPotreeLoader::LoadMetaData() {
    std::cout << "Loading Metadata from " << Path << std::endl;
    std::string error_msg;
    if(!IsValidPotree(Path, error_msg)) throw std::runtime_error(error_msg);
    auto cloud_file = fs::path(Path.c_str()) / "cloud.js";
    MetaData = std::make_unique<vtkPotreeMetaData>();
    MetaData->ReadFromJson(cloud_file.string());
    std::cout << "Loaded Metadata. Point Count: " << MetaData->point_count_ << std::endl;
}

void vtkPotreeLoader::Initialize() {
    if(!MetaData) {
        LoadMetaData();
    }
    auto root_node = std::make_shared<vtkPointHierarchyNode>("", MetaData->bounding_box_);
    LoadNodeHierarchy(root_node);
    RootNode = root_node;
}

void vtkPotreeLoader::LoadNodeHierarchy(const vtkPointHierarchyNodePtr &root_node) const {
    std::queue<vtkPointHierarchyNodePtr> pending_nodes;
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
                if(!node->Children[j]) {
                    auto child = std::make_shared<vtkPointHierarchyNode>(node->GetName() + std::to_string(j), CreateChildBB(node->GetBoundingBox(), j), node);
                    node->Children[j] = child;
                }
                pending_nodes.push(std::dynamic_pointer_cast<vtkPointHierarchyNode>(node->Children[j]));
            }
        }
        f.read(cfg, 5); // read next node
    }

    std::unordered_set<vtkPointHierarchyNode*> seen;    // save the shared_ptr copy overhead and just
                                                // track seen nodes by their address

    while(!pending_nodes.empty()) {
        auto node = pending_nodes.front()->Parent.lock();
        pending_nodes.pop();
        if(node && seen.insert(dynamic_cast<vtkPointHierarchyNode*>(node.get())).second)
            LoadNodeHierarchy(std::dynamic_pointer_cast<vtkPointHierarchyNode>(node));
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
        max[1] -= half_size[1];

    if(index & 4)
        min[0] += half_size[0];
    else
        max[0] -= half_size[0];

    return vtkBoundingBox(min[0], max[0], min[1], max[1], min[2], max[2]);
}

void vtkPotreeLoader::FetchNode(vtkTileHierarchyNodePtr &node) {
    assert(!node->Mapper); // make sure the node is definitely not loaded
    fs::path bin_file = CreateFileName(node->GetName(), ".bin");

    //std::cout << "Loading node data from file " << bin_file.string() << std::endl;
    if (!fs::is_regular_file(bin_file))
    {
        throw std::runtime_error(std::string("file not found: ") + bin_file.string());
    }
    std::size_t size = fs::file_size(bin_file);
    std::ifstream f{bin_file.c_str()};
    if (!f.good())
    {
        throw std::runtime_error(std::string("failed to open file: ") + bin_file.string());
    }
    std::vector<char> data;
    data.resize(size);
    if (!f.read(data.data(), size))
    {
        throw std::runtime_error(std::string("failed to read file: ") + bin_file.string());
    }
    std::size_t point_count = data.size() / MetaData->point_byte_size_;
    //std::cout << "Point count to load " << point_count << std::endl;
    if (point_count == 0)
    {
        // empty nodes do not need a wrapper
        std::lock_guard<std::mutex> lock{node->Mutex};
        node->Mapper = nullptr;
        node->Size = 0;
        return;
    }
    vtkNew<vtkPoints> points;
    points->Allocate(point_count);
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(4);
    colors->SetName("Colors");
    colors->SetNumberOfTuples(points->GetNumberOfPoints());
    std::size_t offset = 0;
    vtkVector3d translate;
    node->BoundingBox.GetMinPoint(translate.GetData());

    for (const std::string& attr : MetaData->point_attributes_) {
        if (attr == "POSITION_CARTESIAN") {
            for (std::size_t i = 0; i < point_count; ++i)
            {
                std::size_t index = offset + i * MetaData->point_byte_size_;
                vtkVector3f point;
                point[0] = *reinterpret_cast<std::uint32_t*>(&data[index + 0])
                           * MetaData->scale_
                           + translate.GetX();
                point[1] = *reinterpret_cast<std::uint32_t*>(&data[index + 4])
                                                                              * MetaData->scale_
                                                                              + translate.GetY();
                point[2] = *reinterpret_cast<std::uint32_t*>(&data[index + 8])
                           * MetaData->scale_
                           + translate.GetZ();
                points->InsertNextPoint(point.GetData());
            }
            //std::cout << "Points size after conversion " << points->GetNumberOfPoints() << std::endl;
        } else if (attr == "COLOR_PACKED")
        {
            for (std::size_t i = 0; i < point_count; ++i)
            {
                std::size_t index = offset + i * MetaData->point_byte_size_;
                vtkVector<float, 4> color;
                color[0] = data[index + 0];//1.f * data[index + 0] / 255.f;
                color[1] = data[index + 1];//1.f * data[index + 1] / 255.f;
                color[2] = data[index + 2];//1.f * data[index + 2] / 255.f;
                color[3] = data[index + 3];//1.f * data[index + 3] / 255.f;
                //std::cout << "Inserting color " << color[0] << ", " << color[1] << ", " << color[2] << ", " << color[3] << std::endl;
                colors->InsertTuple(i, color.GetData());
                //std::cout << "Color after " << colors->GetTuple(i)[0] << ", " << colors->GetTuple(i)[1] << ", " << colors->GetTuple(i)[2] << ", " << colors->GetTuple(i)[3] << std::endl;

            }
        }
        offset += vtkPotreeMetaData::SizeOf(attr);
    }

    if(points->GetNumberOfPoints() == 0) {
        std::cerr << "No POSITION_CARTESIAN data: " << node->GetName() << std::endl;
        std::lock_guard<std::mutex> node_lock{node->Mutex};
        node->Mapper = nullptr;
        node->Size = 0;
        return;
    }
    {
        //std::cout << "Creating the mapper " << std::endl;
        // create the actual mapper
        std::lock_guard<std::mutex> lock{node->Mutex};

        vtkNew<vtkPolyData> polydata;

        polydata->SetPoints(points);
        //polydata->SetVerts(vtk_cells);
        polydata->GetPointData()->SetScalars(colors);
        polydata->GetPointData()->SetActiveScalars("Colors");
        //std::cout << "Done with polydata" << std::endl;
        auto mapper = MakeMapper();
        mapper->SetInputDataObject(polydata);
        //mapper->GetLookupTable()->SetVectorModeToRGBColors();
        //mapper->GetLookupTable().SetNumberOfTableValues(256);

        //mapper->SetColorModeToDirectScalars();
        // update the node with the mapper
        node->Mapper = vtkSmartPointer<vtkMapper>(std::move(mapper));
        node->Size = point_count;

        //std::cout << "Done loading node r" << node->GetName() << std::endl;
    }
}

void vtkPotreeLoader::PrintSelf(ostream &os, vtkIndent indent) {
    vtkTileHierarchyLoader::PrintSelf(os, indent);
}

vtkSmartPointer<vtkMapper> vtkPotreeLoader::MakeMapper() const {
    //std::cout << "Making mapper" << std::endl;

    auto mapper = vtkSmartPointer<vtkPointGaussianMapper>::New();
    assert(mapper->GetReferenceCount() == 1);
    mapper->SetStatic(true);
    mapper->SetEmissive(false);
    mapper->SetScalarModeToUsePointData();
    mapper->SetScaleFactor(0);
    //mapper->SetScaleFactor(0);
    //std::cout << "Made mapper " << std::endl;
    return mapper;
}
