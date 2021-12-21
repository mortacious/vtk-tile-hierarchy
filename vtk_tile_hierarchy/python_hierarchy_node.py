from .vtkTileHierarchy import vtkAttributedTileHierarchyNode
import numpy as np
import vtk


class vtkTileHierarchyNodePython(vtkAttributedTileHierarchyNode):
    def __init__(self, parent=None, bounds=None, num_children=None, **kwargs):
        super().__init__()
        if parent is not None:
            self.parent = parent
        if bounds is None:
            bounds = np.zeros(6, dtype=np.double)
        self.bounds = bounds
        if num_children is not None:
            self.num_children = num_children
        for k, v in kwargs.items():
            self[k] = v

    @property
    def parent(self):
        return self.GetParent()

    @parent.setter
    def parent(self, p):
        self.SetParent(p)

    @property
    def bounds(self):
        return self.GetBoundingBox()

    @bounds.setter
    def bounds(self, b):
        if not isinstance(b, vtk.vtkBoundingBox):
            b = vtk.vtkBoundingBox(np.asarray(b))
        self.SetBoundingBox(b)

    @property
    def num_children(self):
        return self.GetNumChildren()

    @num_children.setter
    def num_children(self, nc):
        self.SetNumChildren(nc)

    @property
    def size(self):
        return self.GetSize()

    @size.setter
    def size(self, s):
        self.SetSize(s)

    @property
    def mapper(self):
        return self.GetMapper()

    @mapper.setter
    def mapper(self, m):
        self.SetMapper(m)

    @property
    def loaded(self):
        return self.IsLoaded()

    def __setitem__(self, key, value):
        self.SetAttribute(key, value)

    def __getitem__(self, item):
        return self.GetAttribute(item)

    def reset(self):
        self.ResetNode()

    def has_child(self, i):
        return self.HasChild(i)

    def get_child(self, i):
        return self.GetChild(i)

    def set_child(self, i, c):
        self.SetChild(i, c)



