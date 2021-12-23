import vtk
import vtk_tile_hierarchy as vtke
from pyvista.plotting import Plotter

path2 = "/home/mortacious/Datasets/ept/ringlok/ept.json"
viewer = Plotter()
loader = vtke.vtkEptLoader(path2)

mapper = vtke.vtkTileHierarchyMapper()
mapper.SetNumThreads(2)
mapper.SetLoader(loader)
mapper.SetPointBudget(10_000_000)
loader.SetCacheSize(20_000_000)
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetRepresentationToPoints()
actor.GetProperty().SetPointSize(3)
viewer.add_actor(actor)
viewer.show()