set(classes
  vtkTileHierarchyNode
  vtkPointHierarchyNode
  vtkTileHierarchyLoaderBase
  vtkTileHierarchyLoader
  vtkPotreeLoader
  vtkTileHierarchyMapper)

if(BUILD_PYTHON)
    list(APPEND classes vtkPythonLoaderBase)
endif()

set(additional_srcs
        internal/vtkPotree1_7DatasetBase.cxx
        internal/vtkPotree1_7DatasetFile.cxx
        internal/vtkPotree1_7DatasetUrl.cxx)

set(additional_headers
        lruCache.h
        priorityQueue.h)

set(HEADER_SPECIFIER NOWRAP_HEADERS)
if(${VTK_VERSION} VERSION_LESS 9.1)
    # vtk 9.0.x has a different name for this
    set(HEADER_SPECIFIER PRIVATE_HEADERS)
endif()

vtk_module_add_module(vtkTileHierarchy
        CLASSES ${classes}
        SOURCES ${additional_srcs}
        ${HEADER_SPECIFIER} ${additional_headers})

vtk_module_link(vtkTileHierarchy
        PUBLIC ${JsonCpp_LIBRARIES} ${CURL_LIBRARIES})