//
// Created by mortacious on 12/23/21.
//

#include <vtk/vtkActor.h>
#include <vtk/vtkNamedColors.h>
#include <vtk/vtkPolyData.h>
#include <vtk/vtkPolyDataMapper.h>
#include <vtk/vtkProperty.h>
#include <vtk/vtkRenderWindow.h>
#include <vtk/vtkRenderWindowInteractor.h>
#include <vtk/vtkInteractorStyleTrackballCamera.h>
#include <vtk/vtkRenderer.h>
#include "vtkTileHierarchyMapper.h"
#include "vtkPotreeLoader.h"

int main() {
    std::string path = "http://5.9.65.151/mschuetz/potree/resources/pointclouds/riegl/retz/cloud.js";
    vtkNew<vtkPotreeLoader> loader;
    loader->SetPath(path);
    loader->SetCacheSize(30000000);

    vtkNew<vtkTileHierarchyMapper> mapper;
    mapper->SetLoader(loader);
    mapper->SetPointBudget(10000000);
    mapper->SetNumThreads(4);

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

    vtkNew<vtkInteractorStyleTrackballCamera> style;
    renderWindowInteractor->SetInteractorStyle(style);
    // Add the actors to the scene
    renderer->AddActor(actor);

    // Render and interact
    renderWindow->SetWindowName("vtkTileHierarchy");
    renderWindow->Render();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}