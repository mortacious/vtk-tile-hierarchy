//
// Created by mortacious on 12/23/21.
//

#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleTerrain.h>

#include <vtkRenderer.h>
#include "vtkTileHierarchyMapper.h"
#include "vtkPotreeLoader.h"

int main() {
    //std::string path = "http://5.9.65.151/mschuetz/potree/resources/pointclouds/riegl/retz/cloud.js";
    std::string path = "/home/mortacious/ros_workspaces/rviz_potree_ws/PotreeConverter/build/ringlok/cloud.js";
    vtkNew<vtkPotreeLoader> loader;
    loader->SetPath(path);
    loader->SetCacheSize(30000000);
    //loader->SetNumThreads(1);

    vtkNew<vtkTileHierarchyMapper> mapper;
    mapper->SetLoader(loader);
    mapper->SetPointBudget(10000000);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetRepresentationToPoints();
    actor->GetProperty()->SetPointSize(3);

    // Create a renderer, render window, and interactor
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(1024, 768);

    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);

    vtkNew<vtkInteractorStyleTerrain> style;
    renderWindowInteractor->SetInteractorStyle(style);
    // Add the actors to the scene
    renderer->AddActor(actor);

    // Render and interact
    renderWindow->SetWindowName("vtkTileHierarchy");
    renderWindow->Render();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}