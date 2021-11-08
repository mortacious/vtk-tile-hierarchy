import vtk
import sys
import vtk_tile_hierarchy as vtke


path = sys.argv[1]

loader = vtke.vtkPotreeLoader()
loader.SetPath(path)
mapper = vtke.vtkTileHierarchyMapper()
mapper.SetLoader(loader)

actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty()

renderer = vtk.vtkRenderer()
renderWindow = vtk.vtkRenderWindow()
renderWindow.SetWindowName("Potree")
renderWindow.SetSize(1024, 768)
renderWindow.AddRenderer(renderer)
renderWindowInteractor = vtk.vtkRenderWindowInteractor()
renderWindowInteractor.SetRenderWindow(renderWindow)

renderWindowInteractor.SetInteractorStyle(vtk.vtkInteractorStyleTrackballCamera())

renderer.AddActor(actor)

renderWindow.Render()
renderWindowInteractor.Start()