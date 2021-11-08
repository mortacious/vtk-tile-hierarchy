set(classes
  vtkTileHierarchyNode
  vtkPointHierarchyNode
  vtkTileHierarchyLoader
  vtkPotreeLoader
  vtkTileHierarchyMapper
  vtkPotreeMetaData
  vtkTileHierarchyLoaderThread)

vtk_module_add_module(vtkTileHierarchy
  CLASSES ${classes})

vtk_module_link(vtkTileHierarchy
        PUBLIC jsoncpp)