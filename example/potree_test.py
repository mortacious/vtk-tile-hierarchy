import vtk
import vtk_tile_hierarchy as vtke
import pyvista as pv

path = "http://5.9.65.151/mschuetz/potree/resources/pointclouds/riegl/retz/cloud.js" #sys.argv[1]
viewer = pv.Plotter()

loader = vtke.vtkPotreeLoader()
loader.SetPath(path)
mapper = vtke.vtkTileHierarchyMapper()
mapper.SetLoader(loader)
mapper.SetPointBudget(10_000_000)
mapper.SetNumThreads(4)
loader.SetCacheSize(30_000_000)
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetRepresentationToPoints()
actor.GetProperty().SetPointSize(3)
viewer.add_actor(actor)
viewer.show()