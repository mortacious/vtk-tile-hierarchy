from .python_hierarchy_loader import vtkPythonHierarchyLoader
from .python_hierarchy_node import vtkTileHierarchyNodePython
from pathlib import Path
import json
import numpy as np
import vtk
from vtk.util.numpy_support import numpy_to_vtk
from itertools import tee


_type_to_numpy_kind = {
    'unsigned': 'u',
    'signed': 'i',
    'float': 'f'
}

_scale_index = {
    'X': 0,
    'Y': 1,
    'Z': 2
}

def pairwise(iterable):
    # pairwise('ABCDEFG') --> AB BC CD DE EF FG
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)

def to_numpy_dtype(type, size):
    return np.dtype(_type_to_numpy_kind[str(type)] + str(size))


class vtkEptLoader(vtkPythonHierarchyLoader):
    def __init__(self, path):
        super().__init__()
        print(path)
        if not self.is_valid(path):
            raise ValueError("Path does not contain an EPT dataset")
        self.path = Path(path).parent

        self.root_node = None
        with open(path, 'r') as f:
            schema = json.load(f)
        if schema['dataType'] != 'binary':
            raise ValueError("Only binary format supported")
        if schema['hierarchyType'] != 'json':
            raise ValueError("Only json format supported")
        self.npoints = schema['points']
        self.span = schema['span']
        self.scale = [1.0, 1.0, 1.0]
        self.offset = [0.0, 0.0, 0.0]
        field_names = []
        field_formats = []
        for dim in schema['schema']:
            name = dim['name']
            if name in _scale_index:
                try:
                    self.scale[_scale_index[name]] = dim['scale']
                except KeyError:
                    pass
                try:
                    self.offset[_scale_index[name]] = dim['offset']
                except KeyError:
                    pass
            field_names.append(name)
            field_formats.append(to_numpy_dtype(dim['type'], dim['size']))

        def replace_fields(names, formats, replace, replacement):
            indices = [names.index(r) for r in replace]
            for i, j in pairwise(indices):
                if j != i+1:
                    raise ValueError("Invalid binary format")
            names[indices[0]] = replacement
            formats[indices[0]] = (formats[indices[0]], len(indices))
            del names[indices[1]:indices[-1]+1]
            del formats[indices[1]:indices[-1]+1]
            return names, formats

        field_names, field_formats = replace_fields(field_names, field_formats, ["X", "Y", "Z"], "Position")
        field_names, field_formats = replace_fields(field_names, field_formats, ["Red", "Green", "Blue"], "Color")

        self.dtype = np.dtype({"names": field_names, "formats": field_formats})

        self.bounds = np.asarray(schema['bounds'], dtype=np.double).reshape(2, 3) - np.asarray(self.offset)

    @staticmethod
    def is_valid(path: Path):
        path = Path(path)
        return path.name == "ept.json"

    @staticmethod
    def index_to_string(index):
        return "-".join((str(i) for i in index))

    @staticmethod
    def string_to_index(name):
        return np.asarray([int(n) for n in name.split('-')], dtype=int)

    def yield_children(self, name):
        new_index = np.flip(self.string_to_index(name))
        new_index[3] += 1 # increase depth by one
        new_index[:3] *= 2 # multiply the rest
        for i in range(8):
            bits = np.unpackbits(np.uint8(i), count=4, bitorder='little')
            tmp_index = np.flip(new_index+bits)
            yield self.index_to_string(tmp_index)

    @staticmethod
    def create_child_bb(bb, child_index):
        min = np.empty(3, dtype=np.double)
        bb.GetMinPoint(min)
        max = np.empty(3, dtype=np.double)
        bb.GetMaxPoint(max)

        half_size = np.empty(3, dtype=np.double)
        bb.GetLengths(half_size)
        half_size /= 2

        if child_index & 1:
            min[2] += half_size[2]
        else:
            max[2] -= half_size[2]

        if child_index & 2:
            min[1] += half_size[1]
        else:
            max[1] -= half_size[1]

        if child_index & 4:
            min[0] += half_size[0]
        else:
            max[0] -= half_size[0]

        return vtk.vtkBoundingBox(min[0], max[0], min[1], max[1], min[2], max[2])

    def parse_hierarchy_data(self, hierarchy_data, node: vtkTileHierarchyNodePython):
        name = node['name']
        node.size = hierarchy_data[name]
        num_nodes, num_points = 1, node.size
        for i, child in enumerate(self.yield_children(name)):
            if child in hierarchy_data:
                child_node = vtkTileHierarchyNodePython(bounds=self.create_child_bb(node.bounds, i),
                                                        parent=node, num_children=8, name=child)
                node.set_child(i, child_node)

                if hierarchy_data[child] < 0:
                    with open(self.path / "ept-hierarchy" / child + ".json", 'r') as f:
                        new_hierarchy_data = json.load(f)
                    num_nodes_sub, num_points_sub = self.parse_hierarchy_data(new_hierarchy_data, child_node)
                else:
                    num_nodes_sub, num_points_sub = self.parse_hierarchy_data(hierarchy_data, child_node)
                num_nodes += num_nodes_sub + 1
                num_points += num_points_sub

        return num_nodes, num_points

    def OnInitialize(self):
        if self.root_node is None:
            name = "0-0-0-0"
            self.root_node = vtkTileHierarchyNodePython(bounds=self.bounds.T, num_children=8, name=name)
            with open(self.path / "ept-hierarchy" / (name + ".json"), 'r') as f:
                hierarchy_data = json.load(f)
            num_nodes, num_points = self.parse_hierarchy_data(hierarchy_data, self.root_node)
            print(f"Loaded a total of {num_nodes} with a total of {num_points} points.")
        return self.root_node

    def OnFetchNode(self, node: vtkTileHierarchyNodePython):
        name = node["name"]
        filepath = self.path / "ept-data" / (name + ".bin")
        data = np.fromfile(filepath, dtype=self.dtype)
        positions = (data["Position"] * self.scale).astype(np.float32)# - self.offset
        colors = (data["Color"] // 256).astype(np.uint8)
        del data
        if positions.shape[0] != colors.shape[0] or positions.shape[0] != node.size:
            node.reset()
            print("Invalid node found. skipping", node.size)
        polydata = vtk.vtkPolyData()
        vtkpoints = vtk.vtkPoints()
        vtkpoints.SetData(numpy_to_vtk(positions, deep=False))
        polydata.SetPoints(vtkpoints)
        colors_array = numpy_to_vtk(colors, deep=False)
        colors_array.SetName("Color")
        polydata.GetPointData().AddArray(colors_array)
        polydata.GetPointData().SetActiveScalars("Color")
        mapper = vtk.vtkPointGaussianMapper()
        mapper.SetStatic(True)
        mapper.SetEmissive(False)
        mapper.SetScalarModeToUsePointData()
        mapper.SetScaleFactor(0)  # Render just points
        mapper.SetInputDataObject(polydata)
        return mapper