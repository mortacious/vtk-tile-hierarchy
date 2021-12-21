set(classes
  vtkTileHierarchyNode
  vtkPointHierarchyNode
  vtkTileHierarchyLoader
  vtkPotreeLoader
  vtkTileHierarchyMapper
  vtkTileHierarchyLoaderThread
  vtkAttributedTileHierarchyNode)

if(BUILD_PYTHON)
    list(APPEND classes vtkPythonLoader)
endif()

set(additional_srcs
        internal/vtkPotree1_7DatasetBase.cxx
        internal/vtkPotree1_7DatasetFile.cxx
        internal/vtkPotree1_7DatasetUrl.cxx)
vtk_module_add_module(vtkTileHierarchy
  CLASSES ${classes} SOURCES ${additional_srcs})

vtk_module_link(vtkTileHierarchy
        PUBLIC jsoncpp ${CURL_LIBRARIES})