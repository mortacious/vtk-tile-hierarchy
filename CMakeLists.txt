cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set (CMAKE_CXX_STANDARD 17)
set(version 1.0)
project(vtkTileHierarchy)


option(BUILD_PYTHON "Build the python wrappers" OFF)
message(STATUS "Rpath is: ${CMAKE_INSTALL_RPATH}")

set(VTK_COMPONENTS
        CommonCore
        Python
        RenderingOpenGL2)

# VTK dependency
find_package(VTK 9.0 REQUIRED
        COMPONENTS
        ${VTK_COMPONENTS}
        )

message(STATUS "VTK python version: ${VTK_PYTHON_VERSION}")
if(${VTK_VERSION} VERSION_LESS 9.0)
    message(FATAL_ERROR "VTK >= 9.0 is REQUIRED")
endif()

find_package(CURL REQUIRED)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
endif()


include(GNUInstallDirs)

set(_shared_default ON)
get_target_property(_vtk_libtype VTK::CommonCore TYPE)
if (_vtk_libtype STREQUAL "STATIC_LIBRARY")
    set(_shared_default OFF)
endif ()

option(BUILD_SHARED_LIBS "Build shared or static libraries" "${_shared_default}")
if(BUILD_PYTHON)
    message(STATUS "Building python module")
    set(OUTPUT_DIR "vtk_tile_hierarchy/lib")
    #message(STATUS "Python include ${Python3_INCLUDE_DIR}")
    include_directories("${Python3_INCLUDE_DIR}")
else()
    set(OUTPUT_DIR ${LIBRARY_OUTPUT_DIRECTORY})
endif()

# First we scan the modules in our project to find out the dependency graph
# between them.
vtk_module_scan(
        # With only 1 module file, this is easier. With more,
        # `vtk_module_find_modules` would be preferred.
        MODULE_FILES      "${CMAKE_CURRENT_SOURCE_DIR}/src/vtk.module"
        # Not building the only module we have is silly.
        REQUEST_MODULES   vtkTileHierarchy
        # Store the list of provided modules from this scan.
        PROVIDES_MODULES  modules
        # Enable the tests for our modules.
        ENABLE_TESTS      OFF
)
# Build the module we just scanned.
vtk_module_build(MODULES ${modules}
        LIBRARY_DESTINATION "${OUTPUT_DIR}"
        INSTALL_HEADERS OFF
        )

if(BUILD_PYTHON)
    # Wrap it with Python.
    vtk_module_wrap_python(
            MODULES         ${modules}
            PYTHON_PACKAGE  "vtk_tile_hierarchy"
            MODULE_DESTINATION "."
            LIBRARY_DESTINATION "${OUTPUT_DIR}"
            # Static Python modules are almost never wanted.
            BUILD_STATIC    OFF
            INSTALL_HEADERS OFF)
endif()