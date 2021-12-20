//
// Created by mortacious on 10/31/21.
//

#include "vtkPotreeLoader.h"
#include "vtkPointHierarchyNode.h"
#include "internal/vtkPotree1_7DatasetFile.h"
#include "internal/vtkPotree1_7DatasetUrl.h"
#include <vtkVector.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkMapper.h>
#include <vtkPointGaussianMapper.h>
#include <vtkIdTypeArray.h>
#include <filesystem>
#include <queue>
#include <array>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkScalarsToColors.h>

namespace fs=std::filesystem;

vtkStandardNewMacro(vtkPotreeLoader);

vtkPotreeLoader::vtkPotreeLoader()
    : Path() {}

void vtkPotreeLoader::Initialize() {
    if(Path.rfind("http://", 0) == 0) {
        Dataset = std::make_unique<vtkPotree1_7DatasetUrl>();
    } else {
        Dataset = std::make_unique<vtkPotree1_7DatasetFile>();
    }
    Dataset->LoadMetaData(Path);
    auto root_node = vtkPointHierarchyNodePtr(nullptr);
    Dataset->LoadNodeHierarchy(root_node);
    RootNode = root_node;
}


void vtkPotreeLoader::FetchNode(vtkTileHierarchyNodePtr &node) {
    //std::lock_guard<std::mutex> node_lock(node->GetMutex());
    auto casted_node = vtkPointHierarchyNode::SafeDownCast(node);

    assert(!casted_node->GetMapper()); // make sure the node is definitely not loaded

    vtkSmartPointer<vtkPolyData> polydata;
    auto point_count = Dataset->LoadNode(casted_node, polydata);
    // create the actual mapper
    //std::lock_guard<std::mutex> lock{node->GetMutex()};

    if(!polydata) {
        casted_node->ResetNode();
        return;
    }

    auto mapper = MakeMapper();
    mapper->SetInputDataObject(polydata);

    casted_node->SetMapper(std::move(mapper));
    casted_node->SetSize(point_count);
}

void vtkPotreeLoader::PrintSelf(ostream &os, vtkIndent indent) {
    vtkTileHierarchyLoader::PrintSelf(os, indent);
}

vtkSmartPointer<vtkMapper> vtkPotreeLoader::MakeMapper() const {
    auto mapper = vtkSmartPointer<vtkPointGaussianMapper>::New();
    assert(mapper->GetReferenceCount() == 1);
    mapper->SetStatic(true);
    mapper->SetEmissive(false);
    mapper->SetScalarModeToUsePointData();
    mapper->SetScaleFactor(0);
    return mapper;
}
