from abc import abstractmethod, ABC
from .vtkTileHierarchy import vtkPythonLoaderBase, vtkTileHierarchyNode
import vtk
from vtkmodules.util.misc import calldata_type
from threading import Thread


class PythonHierarchyLoader(vtkPythonLoaderBase, ABC):
    def __init__(self):
        super().__init__()
        # the callbacks are passed through vtk's event system
        self.AddObserver(self.GetInitializeEvent(), self._on_initialize)
        self.AddObserver(self.GetShutdownEvent(), self._on_shutdown)
        self.AddObserver(self.GetFetchNodeEvent(), self._on_fetch_node)
        self._num_threads = 1
        self._threads = None

    def __del__(self):
       self.shutdown()

    def shutdown(self):
        if self._threads is not None:
            for thread in self._threads:
                thread.join()
            self._threads = None

    def _init_threads(self):
        if self._threads is not None:
            self.Shutdown()
        self._threads = []
        for _ in range(self._num_threads):
            t = Thread(target=self.Run)
            self._threads.append(t)
            t.start()


    @property
    def num_threads(self):
        return 0 if self._threads is None else self._num_threads

    @num_threads.setter
    def num_threads(self, n):
        self._num_threads = n
        if self._threads is not None:
            self._init_threads()

    @abstractmethod
    def on_initialize(self):
        """
        Called on Initialization
        """

    def _on_initialize(self, obj, _):
        self._init_threads()
        ret = self.on_initialize()
        if not isinstance(ret, vtkTileHierarchyNode):
            raise ValueError("Invalid root node")
        obj.SetRootNode(ret)

    def _on_shutdown(self, obj, _):
        self.shutdown()

    @abstractmethod
    def on_fetch_node(self, node):
        """
        Called on FetchNode
        """

    @calldata_type(vtk.VTK_OBJECT)
    def _on_fetch_node(self, _, __, node):
        mapper = self.on_fetch_node(node)
        node.SetMapper(mapper)
