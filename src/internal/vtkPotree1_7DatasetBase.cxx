//
// Created by mortacious on 12/18/21.
//

#include "vtkPotree1_7DatasetBase.h"
#include <json/json.h>
#include <queue>
#include <unordered_set>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVector.h>

#define JSON_GET_EX(Type, Var, Data, Key, Error) \
    do                                           \
    {                                            \
        if (Data[Key].isNull())                  \
            throw std::runtime_error(Error);     \
        Var = Data[Key].as##Type();              \
    } while (0)

#define JSON_GET(Type, Var, Data, Key) \
    JSON_GET_EX(Type, Var, Data, Key, "missing " #Type " key: " Key)

vtkPotree1_7DatasetBase::vtkPotree1_7DatasetBase()
    : point_byte_size_(0) {
}

bool vtkPotree1_7DatasetBase::IsValidPotree(const std::string& path, std::string& error_msg) {
    fs::path p = {path.c_str()};
    error_msg.clear();
    auto fname = p.filename().string();
    if(fname != "cloud.js") {
        error_msg = "Not a Potree 1.7 dataset";
        return false;
    }
    return true;
}

void vtkPotree1_7DatasetBase::LoadMetaData(const std::string& path) {
    std::cout << "Loading Potree 1.7 dataset from: " << path << std::endl;
    std::string error_msg;
    if(!IsValidPotree(path, error_msg)) throw std::runtime_error(error_msg);

    cloud_path_ = fs::path(path).parent_path();

    auto stream = FetchFile(path);

    Json::CharReaderBuilder builder;
    Json::Value data;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    JSONCPP_STRING err;

    if (!reader->parse(stream.c_str(), stream.c_str() + stream.size(), &data,
                       &err)) {
        throw std::runtime_error(std::string("cannot parse meta data: ")
                                 + err);
    }
    JSON_GET(String, octree_dir_, data, "octreeDir");
    JSON_GET(UInt, point_count_, data, "points");
    JSON_GET(UInt, hierarchy_step_size_, data, "hierarchyStepSize");
    JSON_GET(Float, spacing_, data, "spacing");
    JSON_GET(Float, scale_, data, "scale");
    Json::Value bb = data["boundingBox"];
    if (bb.isNull())
        throw std::runtime_error("missing bounding box");
    float lx, ly, lz, ux, uy, uz;
    JSON_GET(Float, lx, bb, "lx");
    JSON_GET(Float, ly, bb, "ly");
    JSON_GET(Float, lz, bb, "lz");
    JSON_GET(Float, ux, bb, "ux");
    JSON_GET(Float, uy, bb, "uy");
    JSON_GET(Float, uz, bb, "uz");
    bounding_box_.SetBounds(lx, ux, ly, uy, lz, uz);
    Json::Value attr = data["pointAttributes"];
    if(attr.isNull()) throw std::runtime_error("missing point attributes");
    point_attributes_.clear();
    point_byte_size_ = 0;
    for (Json::ArrayIndex i = 0; i < attr.size(); ++i)
    {
        std::string val;
        JSON_GET_EX(String, val, attr, i,
                    "invalid point attribute array entry");
        std::size_t sz = SizeOf(val);
        if (sz == 0)
            throw std::runtime_error("unsupported point attribute: " + val);
        point_attributes_.push_back(val);
        point_byte_size_ += sz;
    }


    std::cout << "Loaded Metadata. Point Count: " << point_count_ << std::endl;
}

std::string vtkPotree1_7DatasetBase::CreateFileName(const std::string &name, const std::string &extension) const {
    fs::path octree_dir = cloud_path_ / octree_dir_;
    //std::cout << "Octree dir" << octree_dir << std::endl;
    fs::path result;
    std::size_t levels = name.length() / hierarchy_step_size_;

    for(std::size_t i = 0; i < levels; ++i) {
        result /= name.substr(i * hierarchy_step_size_, hierarchy_step_size_);
    }

    result /= std::string("r") + name + extension;
    if(fs::is_regular_file(octree_dir / "u" / result))
        return octree_dir / "u" / result;
    return octree_dir / "r" / result;
}

void vtkPotree1_7DatasetBase::LoadNodeHierarchy(vtkPointHierarchyNodePtr &root_node) const {
    root_node = vtkPointHierarchyNodePtr::New();
    root_node->SetBoundingBox(bounding_box_);
    root_node->SetNumChildren(8);
    std::cout << "Loading hierarchy" << std::endl;
    size_t points_loaded = 0;
    size_t nodes_loaded = 0;
    LoadNodeHierarchy(root_node, points_loaded, nodes_loaded);
    std::cout << "\r" <<std::endl;
    std::cout << "Done. Loaded a total of " << nodes_loaded << " nodes." << std::endl;
}

void vtkPotree1_7DatasetBase::LoadNodeHierarchy(vtkPointHierarchyNodePtr &root_node, size_t &points_loaded,
                                                size_t &nodes_loaded) const {
    std::queue<vtkPointHierarchyNodePtr> pending_nodes;
    pending_nodes.push(root_node);

    fs::path hrc_file = CreateFileName(root_node->GetName(), ".hrc");
    auto data = FetchFile(hrc_file.string());
    std::cout << 0.0 << std::flush;
    for(size_t pos = 0; pos < data.size(); pos+=5) {
        auto node = pending_nodes.front();
        pending_nodes.pop();
        points_loaded += *((uint32_t*)&data[pos + 1]);
        std::cout << "\r" << static_cast<double>(points_loaded)/point_count_ * 100 << std::flush;
        for(int j=0; j<8; ++j) {
            if(data[pos] & (1 << j)) {
                if(!node->HasChild(j)) {
                    auto child = vtkPointHierarchyNodePtr::New();
                    child->SetBoundingBox(CreateChildBB(node->GetBoundingBox(), j));
                    child->SetName(node->GetName() + std::to_string(j));
                    child->SetParent(node);
                    child->SetNumChildren(8);
                    node->SetChild(j, std::move(child));
                    nodes_loaded++;
                }
                pending_nodes.push(vtkPointHierarchyNode::SafeDownCast(node->GetChild(j)));
            }
        }
    }

    std::unordered_set<vtkPointHierarchyNode*> seen;    // save the shared_ptr copy overhead and just
    // track seen nodes by their address

    while(!pending_nodes.empty()) {
        auto node = pending_nodes.front()->GetParent();
        pending_nodes.pop();
        if(node && seen.insert(dynamic_cast<vtkPointHierarchyNode*>(node.GetPointer())).second) {
            vtkPointHierarchyNodePtr node_ptr(vtkPointHierarchyNode::SafeDownCast(node));
            LoadNodeHierarchy(node_ptr, points_loaded, nodes_loaded);
        }
    }
}

vtkBoundingBox vtkPotree1_7DatasetBase::CreateChildBB(const vtkBoundingBox &parent, int index) {
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

size_t vtkPotree1_7DatasetBase::LoadNode(const vtkPointHierarchyNodePtr &node,
                                         vtkSmartPointer<vtkPolyData> &polydata) const {
    fs::path bin_file = CreateFileName(node->GetName(), ".bin");

    auto data = FetchFile(bin_file);
    if(data.empty()) {
        throw std::runtime_error(std::string("failed to read file: ") + bin_file.string());
    }
    std::size_t point_count = data.size() / point_byte_size_;
    if (point_count == 0)
    {
        // empty nodes do not need a wrapper
        return 0;
    }

    vtkNew<vtkPoints> points;
    points->Allocate(point_count);
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(4);
    colors->SetName("Colors");
    colors->SetNumberOfTuples(points->GetNumberOfPoints());
    std::size_t offset = 0;
    vtkVector3d translate;
    node->GetBoundingBox().GetMinPoint(translate.GetData());

    for (const std::string& attr : point_attributes_) {
        if (attr == "POSITION_CARTESIAN") {
            for (std::size_t i = 0; i < point_count; ++i)
            {
                std::size_t index = offset + i * point_byte_size_;
                vtkVector3f point;
                point[0] = *reinterpret_cast<std::uint32_t*>(&data[index + 0])
                           * scale_
                           + translate.GetX();
                point[1] = *reinterpret_cast<std::uint32_t*>(&data[index + 4])
                           * scale_
                           + translate.GetY();
                point[2] = *reinterpret_cast<std::uint32_t*>(&data[index + 8])
                           * scale_
                           + translate.GetZ();
                points->InsertNextPoint(point.GetData());
            }
        } else if (attr == "COLOR_PACKED")
        {
            for (std::size_t i = 0; i < point_count; ++i)
            {
                std::size_t index = offset + i * point_byte_size_;
                vtkVector<float, 4> color;
                color[0] = data[index + 0];//1.f * data[index + 0] / 255.f;
                color[1] = data[index + 1];//1.f * data[index + 1] / 255.f;
                color[2] = data[index + 2];//1.f * data[index + 2] / 255.f;
                color[3] = data[index + 3];//1.f * data[index + 3] / 255.f;
                colors->InsertTuple(i, color.GetData());

            }
        }
        offset += SizeOf(attr);
    }

    if(points->GetNumberOfPoints() == 0) {
        std::cerr << "No POSITION_CARTESIAN data: " << node->GetName() << std::endl;
        return 0;
    }

    polydata = vtkSmartPointer<vtkPolyData>::New();

    polydata->SetPoints(points);
    polydata->GetPointData()->SetScalars(colors);
    polydata->GetPointData()->SetActiveScalars("Colors");
    return point_count;
}


size_t vtkPotree1_7DatasetBase::SizeOf(const std::string& attr) {
    if (attr == "POSITION_CARTESIAN")
        return 12;
    if (attr == "COLOR_PACKED")
        return 4;
    if (attr == "INTENSITY")
        return 2;
    if (attr == "CLASSIFICATION")
        return 1;
    if (attr == "RETURN_NUMBER")
        return 1;
    if (attr == "NUMBER_OF_RETURNS")
        return 1;
    if (attr == "SOURCE_ID")
        return 2;
    if (attr == "GPS_TIME")
        return 8;
    if (attr == "NORMAL_SPHEREMAPPED")
        return 2;
    if (attr == "NORMAL_OCT16")
        return 2;
    if (attr == "NORMAL")
        return 12;
    return 0;
}
