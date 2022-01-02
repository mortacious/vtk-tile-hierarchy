import vtk
import vtk_tile_hierarchy as vtke
from pyvista.plotting import Plotter
path2 = "/home/mortacious/Datasets/ept/ringlok/ept.json"
loader = vtke.vtkEptLoader(path2)
mapper = vtke.vtkTileHierarchyMapper()
#mapper.SetUseTimer(False)
mapper.SetLoader(loader)
mapper.SetPointBudget(10_000_000)
loader.SetCacheSize(30_000_000)
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetRepresentationToPoints()
actor.GetProperty().SetPointSize(3)

viewer = Plotter()
viewer.add_actor(actor)
viewer.show()
