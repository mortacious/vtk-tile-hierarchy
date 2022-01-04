import vtk
import vtk_tile_hierarchy as vtke
#from pyvista.plotting import Plotter
from hyperspace.viewer import Viewer
#path2 = "/home/mortacious/Datasets/ept/ringlok/ept.json"
path2 = "https://na-c.entwine.io/private/hanover-filtered/ept.json"
loader = vtke.EptLoader(path2)
loader.num_threads = 4
mapper = vtke.vtkTileHierarchyMapper()
mapper.SetUseTimer(False)
mapper.SetLoader(loader)
mapper.SetPointBudget(10_000_000)
loader.SetCacheSize(30_000_000)
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetRepresentationToPoints()
actor.GetProperty().SetPointSize(3)

viewer = Viewer(show=False)
viewer.add_actor(actor)
viewer.show()
