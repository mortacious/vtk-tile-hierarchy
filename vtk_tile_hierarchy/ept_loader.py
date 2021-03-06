from .python_hierarchy_loader import PythonHierarchyLoader
from .python_hierarchy_node import TileHierarchyNodePython
from pathlib import Path
import json
import numpy as np
import vtk
from vtk.util.numpy_support import numpy_to_vtk
from itertools import tee
import urllib.parse as urlparse
import gzip

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


class EptLoader(PythonHierarchyLoader):
    def __init__(self, path):
        super().__init__()

        self.url = urlparse.urlparse(path)
        if self.url.scheme != '':
            try:
                import pycurl
            except ImportError:
                raise ValueError("Remote datasets require the 'pycurl' library to be installed")

        path = self.url.path
        if not self.is_valid(path):
            raise ValueError("Path does not contain an EPT dataset")
        self.path = Path(path).parent

        self.root_node = None
        schema = self._fetch_json_file(path, add_extension=False)
        if schema['dataType'] == 'binary':
            self.binary_file_extension = '.bin'
        elif schema['dataType'] == 'laszip':
            self.binary_file_extension = '.laz'
        else:
            raise ValueError(f"Unsupported dataType {schema['dataType']}")

        if schema['hierarchyType'] == 'json':
            self.json_file_extension = '.json'
        elif schema['hierarchyType'] == 'gzip':
            self.json_file_extension = '.json.gz'
        else:
            raise ValueError(f"Unsupported hierarchyType {schema['hierarchyType']}")

        try:
            self.npoints = schema['points']
        except KeyError:
            self.npoints = schema['numPoints']
        self.span = schema.get('span', 256)
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

        field_names, field_formats = replace_fields(field_names, field_formats, ["X", "Y", "Z"], "Position")
        field_names, field_formats = replace_fields(field_names, field_formats, ["Red", "Green", "Blue"], "Color")

        self.dtype = np.dtype({"names": field_names, "formats": field_formats})

        bounds = vtk.vtkBoundingBox((np.asarray(schema['bounds'], dtype=np.double).reshape(2, 3)- np.asarray(self.offset)).T.ravel())
        self.SetBoundingBox(bounds)

    def _fetch_pycurl(self, path):
        import pycurl
        import certifi
        from io import BytesIO
        path = str(path)
        url = self.url._replace(path=path).geturl()
        buffer = BytesIO()
        c = pycurl.Curl()
        c.setopt(c.URL, url)
        c.setopt(c.NOPROGRESS, 1)
        c.setopt(c.FAILONERROR, True)
        c.setopt(c.WRITEDATA, buffer)
        c.setopt(c.CAINFO, certifi.where())
        c.perform()
        c.close()
        buffer.seek(0)
        return buffer

    def _fetch_json_file(self, path, add_extension=True):
        """

        :param path:
        :return:
        """
        path = Path(path)
        if add_extension:
            path = path.with_suffix(self.json_file_extension)
            extension = self.json_file_extension
        else:
            extension = '.json'
        if self.url.scheme == '':
            if extension == '.json':
                # local file
                with open(path, 'r') as f:
                    return json.load(f)
            else:
                with gzip.open(path, 'r') as f:
                    return json.load(f)
        else:
            buffer = self._fetch_pycurl(path)
            data = buffer.getvalue()
            if add_extension and extension == '.json.gz':
                data = gzip.decompress(data)
            return json.loads(data)

    def _process_lasdata(self, lasdata):
        dtype = lasdata.points.array.dtype
        names = list(dtype.names)
        formats = [d[1] for d in dtype.descr]

        names, formats = replace_fields(names, formats, ["X", "Y", "Z"], "Position")
        names, formats = replace_fields(names, formats, ["red", "green", "blue"], "Color")
        dtype = np.dtype({"names": names, "formats": formats})

        data = lasdata.points.array
        data = data.view(dtype)
        return data

    def _fetch_binary_file(self, path, dtype=None, add_extension=True):
            """

            :param path:
            :param dtype:
            :return:
            """
            path = Path(path)
            if add_extension:
                path = path.with_suffix(self.binary_file_extension)
            if self.url.scheme == '':
                if self.binary_file_extension == '.bin':
                    return np.fromfile(path, dtype=dtype)
                else:
                    import laspy
                    lasdata = laspy.read(path)
                    return self._process_lasdata(lasdata)
            else:
                buffer = self._fetch_pycurl(path)
                if self.binary_file_extension == '.bin':
                    bin_string = buffer.getvalue()
                    return np.frombuffer(bin_string, dtype)
                else:
                    import laspy
                    lasdata = laspy.read(buffer)
                    return self._process_lasdata(lasdata)


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
        new_index[3] += 1  # increase depth by one
        new_index[:3] *= 2  # multiply the rest
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

    def parse_hierarchy_data(self, node: TileHierarchyNodePython):
        if node.size >= 0:
            return

        hierarchy_data = node["hierarchy"]
        name = node['name']
        del node["hierarchy"]  # remove the hierarchy data as the node is expanded
        node.size = hierarchy_data[name]
        for i, child in enumerate(self.yield_children(name)):
            if child in hierarchy_data:
                child_node = TileHierarchyNodePython(bounds=self.create_child_bb(node.bounds, i),
                                                     parent=node, num_children=8, name=child)
                node.set_child(i, child_node)
                child_node.size = -1
                if hierarchy_data[child] < 0:
                    # replace the hierarchy data for this node with the new sub hierarchy
                    new_hierarchy_data = self._fetch_json_file(self.path / "ept-hierarchy" / child)
                    child_node['hierarchy'] = new_hierarchy_data
                else:
                    child_node['hierarchy'] = hierarchy_data

    def on_initialize(self):
        if self.root_node is None:
            name = "0-0-0-0"
            self.root_node = TileHierarchyNodePython(bounds=self.GetBoundingBox(), num_children=8, name=name)
            hierarchy_data = self._fetch_json_file(self.path / "ept-hierarchy" / name)
            self.root_node["hierarchy"] = hierarchy_data
            self.root_node.size = -1
        return self.root_node

    def on_fetch_node(self, node: TileHierarchyNodePython):
        name = node["name"]

        if node.size < 0:  # the hierarchy has not been parsed for this node so load it first
            self.parse_hierarchy_data(node)

        filepath = self.path / "ept-data" / name
        data = self._fetch_binary_file(filepath, dtype=self.dtype)
        positions = (data["Position"] * self.scale).astype(np.float32)# + self.offset
        colors = data["Color"]
        color_max = np.max(colors)
        if color_max > 256:
            colors //= 256
        colors = colors.astype(np.uint8)

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