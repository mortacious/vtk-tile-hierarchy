import vtk
import vtk_tile_hierarchy as vtke
from pyvista.plotting import Plotter

path2 = "https://na-c.entwine.io/private/hanover-filtered/ept.json"
#path2 = "https://ept-m3dc-pmsp.s3-sa-east-1.amazonaws.com/ept.json"

loader = vtke.EptLoader(path2)
loader.num_threads = 24
mapper = vtke.vtkTileHierarchyMapper()
mapper.SetUseTimer(False)
mapper.SetLoader(loader)
mapper.SetPointBudget(10_000_000)
loader.SetCacheSize(30_000_000)
actor = vtk.vtkActor()
actor.SetMapper(mapper)
actor.GetProperty().SetRepresentationToPoints()
actor.GetProperty().SetPointSize(3)

viewer = Plotter()
viewer.add_actor(actor)
#viewer.enable_eye_dome_lighting()
viewer.iren.interactor.SetInteractorStyle(vtke.InteractorStylePivot())

viewer.reset_camera()
viewer.show()
