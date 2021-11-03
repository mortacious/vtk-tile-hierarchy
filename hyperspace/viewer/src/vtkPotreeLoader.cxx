//
// Created by mortacious on 10/31/21.
//

#include "vtkPotreeLoader.h"
#include "vtkPotreeMetaData.h"
#include "vtkPotreeNode.h"
#include <vtkBoundingBox.h>
#include <vtkVector.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>
#include <vtkIdTypeArray.h>
#include <filesystem>
#include <queue>
#include <array>
#include <unordered_set>
#include <vtkFloatArray.h>

namespace fs=std::filesystem;

vtkStandardNewMacro(vtkPotreeLoader);


size_t PotreeNodeSize::operator()(const vtkPotreeNodePtr &k, const std::pair<vtkSmartPointer<vtkMapper>, size_t> &v) const {
    return k->GetPointCount();
}

vtkPotreeLoader::vtkPotreeLoader()
    : Path(""), Cache(5000000)
{
    std::cout << "Cache size on init is " << Cache.cache_size() << " num entries " << Cache.num_entries() << std::endl;

    MapperTemplate = vtkSmartPointer<vtkPolyDataMapper>::New();
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

void vtkPotreeLoader::SetTemplateMapper(vtkMapper *mapper) {
    MapperTemplate.TakeReference(mapper);
}

vtkMapper * vtkPotreeLoader::GetTemplateMapper() {
    return MapperTemplate.Get();
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

vtkPotreeNodePtr vtkPotreeLoader::LoadHierarchy() {
    if(!MetaData) {
        LoadMetaData();
    }
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
        max[1] -= half_size[1];

    if(index & 4)
        min[0] += half_size[0];
    else
        max[0] -= half_size[0];

    return vtkBoundingBox(min[0], max[0], min[1], max[1], min[2], max[2]);
}

void vtkPotreeLoader::LoadNode(vtkPotreeNodePtr &node, bool recursive) {
    std::unique_lock<std::mutex> lock{node->Mutex};

    if(Cache.exist(node)) {
        auto val = Cache.get(node);
        node->Mapper = val.first;
        node->PointCount = val.second;
        std::cout << "Restoring node r" << node->GetName() << "with " << node->GetPointCount() << " points from cache";
        //std::cout << "Cached node has " << node->PointCount << " points" << std::endl;
        node->Loaded = true;
        Cache.erase(node); // Remove the node from cache as long as it is needed by the mapper
        lock.unlock();
    } else {
        lock.unlock();
        LoadNodeFromFile(node);
    }
    // recursively load the nodes below
    if (recursive)
    {
        for (vtkPotreeNodePtr& child : node->Children)
        {
            if (child)
                LoadNode(child, true);
        }
    }
}

void vtkPotreeLoader::LoadNodeFromFile(vtkPotreeNodePtr &node) {
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
        std::cout << "Got empty node: r" << node->GetName() << std::endl;
        throw std::runtime_error("Empty node");
        std::lock_guard<std::mutex> lock{node->Mutex};
        node->Mapper = nullptr;
        node->PointCount = 0;
        node->Loaded = true;
        return;
    }
    vtkNew<vtkPoints> points;
    points->Allocate(point_count);
    vtkNew<vtkFloatArray> colors;
    colors->SetNumberOfComponents(4);
    colors->SetName("colors");
    colors->SetNumberOfTuples(points->GetNumberOfPoints());
    std::size_t offset = 0;
    vtkVector3d translate;
    node->BoundingBox.GetMinPoint(translate.GetData());

    for (const std::string& attr : MetaData->point_attributes_) {
        //std::cout << "Point attribute " << attr << std::endl;
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
                color[0] = 1.f * data[index + 0] / 255.f;
                color[1] = 1.f * data[index + 1] / 255.f;
                color[2] = 1.f * data[index + 2] / 255.f;
                color[3] = 1.f * data[index + 3] / 255.f;
                //std::cout << "Inserting color " << color[0] << ", " << color[1] << ", " << color[2] << ", " << color[3] << std::endl;
                colors->InsertTuple(i, color.GetData());
                //std::cout << "Color after " << colors->GetTuple(i)[0] << ", " << colors->GetTuple(i)[1] << ", " << colors->GetTuple(i)[2] << ", " << colors->GetTuple(i)[3] << std::endl;

            }
        }
        offset += vtkPotreeMetaData::SizeOf(attr);
    }

    if(points->GetNumberOfPoints() == 0) {
        std::cerr << "No POSITION_CARTESIAN data: " << node->GetName() << std::endl;
        std::lock_guard<std::mutex> lock{node->Mutex};
        node->Mapper = nullptr;
        node->PointCount = 0;
        node->Loaded = true;
        return;
    }
    {
        // create the actual mapper
        std::lock_guard<std::mutex> lock{node->Mutex};
        node->PointCount = point_count;

        vtkNew<vtkPolyData> polydata;

        // Add cells
        vtkSmartPointer<vtkCellArray> vtk_cells = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkIdTypeArray> cells = vtkSmartPointer<vtkIdTypeArray>::New();
        vtkIdType* cell_buf = new vtkIdType[point_count * 2];
        for(size_t i = 0; i < point_count; i++)
        {
            size_t i2 = 2 * i;
            cell_buf[i2 + 0] = static_cast<vtkIdType>(1);
            cell_buf[i2 + 1] = i;
        }
        cells->SetVoidArray(cell_buf, point_count * 2, 0, vtkIdTypeArray::VTK_DATA_ARRAY_DELETE);
        vtk_cells->SetCells(point_count, cells);

        polydata->SetPoints(points);
        polydata->SetVerts(vtk_cells);
        polydata->GetPointData()->SetScalars(colors);
        polydata->GetPointData()->SetActiveScalars("colors");
        vtkMapper* mapper = MapperTemplate->NewInstance();
        assert(mapper->GetReferenceCount() == 1);
        mapper->ShallowCopy(MapperTemplate);
        mapper->SetInputDataObject(polydata);
        mapper->SetStatic(true);
        // update the node with the mapper
        node->Mapper.TakeReference(mapper);
        node->Loaded = true;
    }
}

void vtkPotreeLoader::UnloadNode(vtkPotreeNodePtr &node, bool recursive) {
    std::unique_lock<std::mutex> lock{node->Mutex};
    if(node->IsLoaded() && !Cache.exist(node)) {
        // cache this node if it is loaded and not in the cache
        Cache.put(node, std::make_pair(node->Mapper, node->PointCount));
    }
    node->Loaded = false;

    node->Mapper = nullptr; // delete the mapper and it's ressources
    lock.unlock();
    if(recursive) {
        for (vtkPotreeNodePtr & child : node->Children) {
            if(child) {
                UnloadNode(child, true);
            }
        }
    }
}

bool vtkPotreeLoader::IsCached(vtkPotreeNodePtr &node) const {
    return Cache.exist(node);
}

void vtkPotreeLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}
