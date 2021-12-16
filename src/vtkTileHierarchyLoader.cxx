//
// Created by mortacious on 11/3/21.
//

#include "vtkTileHierarchyLoader.h"
#include "vtkTileHierarchyNode.h"
#include <vtkBoundingBox.h>
#include <vtkObjectFactory.h>
#include <vtkMapper.h>
#include <vtkDataSetMapper.h>

size_t vtkTileHierarchyLoader::TileTreeNodeSize::operator()(const vtkTileHierarchyNodePtr &k, const std::pair<vtkSmartPointer<vtkMapper>, size_t> &v) const {
    return k->GetSize();
}

vtkTileHierarchyLoader::vtkTileHierarchyLoader()
    : MapperTemplate(vtkDataSetMapper::New()), RootNode(nullptr), Cache(15000000) {
    MapperTemplate->SetColorModeToMapScalars();
    MapperTemplate->SetScalarModeToUsePointData();
    MapperTemplate->SetStatic(true);

}

void vtkTileHierarchyLoader::SetMapperTemplate(vtkMapper *mapper) {
    MapperTemplate.TakeReference(mapper);
}

vtkMapper * vtkTileHierarchyLoader::GetMapperTemplate() {
    return MapperTemplate.Get();
}

void vtkTileHierarchyLoader::PrintSelf(ostream &os, vtkIndent indent) {
    Superclass::PrintSelf(os, indent);
}

bool vtkTileHierarchyLoader::IsCached(const vtkTileHierarchyNodePtr &node) const {
    return Cache.exist(node);
}

vtkTileHierarchyNodePtr vtkTileHierarchyLoader::GetRootNode() {
    if(!RootNode) Initialize();
    return RootNode;
}

vtkMapper * vtkTileHierarchyLoader::MakeMapper() const {
    vtkMapper* mapper = MapperTemplate->NewInstance();
    assert(mapper->GetReferenceCount() == 1);
    mapper->ShallowCopy(MapperTemplate);
    mapper->SetStatic(true);
    return mapper;
}
