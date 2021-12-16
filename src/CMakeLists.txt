set(classes
  vtkTileHierarchyNode
  vtkPointHierarchyNode
  vtkTileHierarchyLoader
  vtkPotreeLoader
  vtkTileHierarchyMapper
  vtkPotreeMetaData
  vtkTileHierarchyLoaderThread)

set(additional_srcs
        fileDownload.cxx)
vtk_module_add_module(vtkTileHierarchy
  CLASSES ${classes} SOURCES ${additional_srcs})

vtk_module_link(vtkTileHierarchy
        PUBLIC jsoncpp ${CURL_LIBRARIES})