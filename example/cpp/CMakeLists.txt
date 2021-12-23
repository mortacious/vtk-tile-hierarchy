cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(potree_test)

find_package(VTK 9.0 REQUIRED COMPONENTS CommonCore
        InteractionStyle
        RenderingOpenGL2
        )
if(${VTK_VERSION} VERSION_LESS 9.0)
    message(FATAL_ERROR "VTK >= 9.0 is REQUIRED")
endif()

find_package(vtkTileHierarchy REQUIRED)
include_directories(${vtkTileHierarchy_INCLUDE_DIRS})

add_executable(potree_test potree_test.cpp)
target_link_libraries(potree_test ${VTK_LIBRARIES} ${vtkTileHierarchy_LIBRARIES})

vtk_module_autoinit(
        TARGETS potree_test
        MODULES ${VTK_LIBRARIES}
)