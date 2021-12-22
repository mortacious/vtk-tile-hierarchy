from abc import abstractmethod, ABC
from .vtkTileHierarchy import vtkPythonLoader, vtkTileHierarchyNode
import vtk
from vtkmodules.util.misc import calldata_type


class vtkPythonHierarchyLoader(vtkPythonLoader, ABC):
    def __init__(self):
        # the callbacks are passed through vtk's event system
        self.AddObserver(self.GetInitializeEvent(), self._on_initialize)
        self.AddObserver(self.GetFetchNodeEvent(), self._on_fetch_node)

    @abstractmethod
    def OnInitialize(self):
        """
        Called on Initialization
        """

    def _on_initialize(self, obj, _):
        ret = self.OnInitialize()
        if not isinstance(ret, vtkTileHierarchyNode):
            raise ValueError("Invalid root node")
        obj.SetRootNode(ret)

    @abstractmethod
    def OnFetchNode(self, node):
        """
        Called on FetchNode
        """

    @calldata_type(vtk.VTK_OBJECT)
    def _on_fetch_node(self, _, __, node):
        mapper = self.OnFetchNode(node)
        node.SetMapper(mapper)
