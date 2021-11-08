cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set (CMAKE_CXX_STANDARD 17)
set(version 1.0)
project(vtkTileHierarchy)

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


include(GNUInstallDirs)

set(_shared_default ON)
get_target_property(_vtk_libtype VTK::CommonCore TYPE)
if (_vtk_libtype STREQUAL "STATIC_LIBRARY")
    set(_shared_default OFF)
endif ()

option(BUILD_SHARED_LIBS "Build shared or static libraries" "${_shared_default}")
set(OUTPUT_DIR ".")
message("LIBRARY_OUTPUT_DIR ${OUTPUT_DIR}/vtk_extensions/lib  | ${CMAKE_INSTALL_LIBDIR}")

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
        LIBRARY_DESTINATION "${OUTPUT_DIR}/vtk_tile_hierarchy/lib"
        INSTALL_HEADERS OFF
        )

# Wrap it with Python.
vtk_module_wrap_python(
        MODULES         ${modules}
        PYTHON_PACKAGE  "vtk_tile_hierarchy"
        MODULE_DESTINATION "${OUTPUT_DIR}"
        LIBRARY_DESTINATION "${OUTPUT_DIR}/vtk_tile_hierarchy/lib"
        # Static Python modules are almost never wanted.
        BUILD_STATIC    OFF
        INSTALL_HEADERS OFF)

# Create an `__init__.py` for the wrapped module.
#file(WRITE "${CMAKE_INSTALL_PREFIX}/vtk_extensions/__init__.py" "from .vtkHyperspaceExtensions import *")