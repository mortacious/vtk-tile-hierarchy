from abc import abstractmethod, ABC
import vtk_tile_hierarchy as _vtkt
import vtk
from vtkmodules.util.misc import calldata_type


class vtkPythonHierarchyLoader(ABC):
    def __init__(self):
        self._internal_loader = _vtkt.vtkGenericLoader()
        # the callbacks are passed through vtk's event system
        self._internal_loader.AddObserver(self._internal_loader.GetInitializeEvent(), self._on_initialize)
        self._internal_loader.AddObserver(self._internal_loader.GetFetchNodeEvent(), self._on_fetch_node)

    @abstractmethod
    def OnInitialize(self):
        """
        Called on Initialization
        """
        return None

    def _on_initialize(self, obj, evnt):
        ret = self.OnInitialize()
        if ret is None:
            raise ValueError("Invalid hierarchy")
        obj.SetRootNode(ret)

    def OnFetchNode(self, node):
        """
        Called on FetchNode
        """

    @calldata_type(vtk.VTK_OBJECT)
    def _on_fetch_node(self, obj, evnt, calldata):
        self.OnFetchNode(calldata)