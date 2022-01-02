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
        minMaxHeap.h
        priorityQueue.h)

vtk_module_add_module(vtkTileHierarchy
        CLASSES ${classes}
        SOURCES ${additional_srcs}
        HEADERS ${additional_headers})

vtk_module_link(vtkTileHierarchy
        PUBLIC ${JsonCpp_LIBRARIES} ${CURL_LIBRARIES})