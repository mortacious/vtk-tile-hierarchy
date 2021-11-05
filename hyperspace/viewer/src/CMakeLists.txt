set(classes
  vtkOpenGLGridMapper
  vtkTileHierarchyNode
  vtkPointHierarchyNode
  vtkTileHierarchyLoader
  vtkPotreeLoader
  vtkTileHierarchyMapper
  vtkPotreeMetaData
  vtkTileHierarchyLoaderThread)

vtk_module_add_module(Hyperspace::vtkExtensions
  CLASSES ${classes})

vtk_module_link(Hyperspace::vtkExtensions
        PUBLIC jsoncpp)