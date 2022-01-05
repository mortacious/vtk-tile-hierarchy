import vtk
from vtk.util import numpy_support as VN
import numpy as np
from .default_interactor_keybindings import DefaultInteractorKeybindsMixin


def _normalize(v):
    v = np.asarray(v)
    norm = np.linalg.norm(v)
    if norm == 0:
        return v
    return v / norm


class InteractorStylePivot(DefaultInteractorKeybindsMixin, vtk.vtkInteractorStyleTrackballCamera):
    """
    Custom interactor style that does every interaction relative to a pivot point under the mouse pointer.
    The pivot point is picked on every mouse click using a window size dependent tolerance. If there is no point under
    the mouse the previous pivot point is used.
    """
    def __init__(self):
        vtk.vtkInteractorStyleTrackballCamera.__init__(self)
        DefaultInteractorKeybindsMixin.__init__(self)

        self.AddObserver("LeftButtonPressEvent", self._left_button_press_event)
        self.AddObserver("LeftButtonReleaseEvent", self._left_button_release_event)
        self.AddObserver("RightButtonPressEvent", self._right_button_press_event)
        self.AddObserver("RightButtonReleaseEvent", self._right_button_release_event)
        self.AddObserver("MouseMoveEvent", self._mouse_move_event)
        self.AddObserver("MiddleButtonPressEvent", self._middle_button_press_event)
        self.AddObserver("MiddleButtonReleaseEvent", self._middle_button_release_event)
        self.AddObserver("MouseWheelForwardEvent", self._mousewheel_forward_event)
        self.AddObserver("MouseWheelBackwardEvent", self._mousewheel_backward_event)

        # create focus sphere actor
        _sphere = vtk.vtkSphereSource()
        _sphere.SetThetaResolution(6)
        _sphere.SetPhiResolution(6)
        _sphere_mapper = vtk.vtkPolyDataMapper()
        _sphere_mapper.SetInputConnection(_sphere.GetOutputPort())
        del _sphere

        self._pivot_sphere = vtk.vtkActor()
        self._pivot_sphere.SetMapper(_sphere_mapper)
        self._pivot_sphere.GetProperty().SetColor(1, 1, 1)
        self._pivot_sphere.GetProperty().SetRepresentationToWireframe()
        del _sphere_mapper

        self.left_down = False
        self.right_down = False
        self.state = 0
        self.pivot = [0, 0, 0]
        self.tolerance = 0.01

    @property
    def tolerance(self):
        """
        Tolerance of the mouse pointer in percentage of the window diagonal size.
        Returns
        -------
        tolerance: float
            the tolerance parameter

        """
        return self.__tolerance

    @tolerance.setter
    def tolerance(self, tol):
        """
        Tolerance of the mouse pointer in percentage of the window diagonal size.
        Parameters
        ----------
        tol: float
            the new tolerance
        """
        self.__tolerance = tol

    @property
    def translation_plane_normal(self):
        """
        EXPERIMENTAL
        The plane normal used for the translation
        Returns
        -------
        translation_plane_normal: the normal vector
        """
        return getattr(self, '_translation_plane_normal', None)

    @translation_plane_normal.setter
    def translation_plane_normal(self, plane):
        """
        EXPERIMENTAL
        The plane normal used for the translation

        Parameters
        ----------
        plane: array[3] the plane normal
        """
        plane = np.asarray(plane).ravel()
        if len(plane) != 3:
            raise ValueError("Plane must be a normalized vector of 3 components")
        self._translation_plane_normal = plane

    def _set_pivot(self, event_pos):
        """
        Pick a new pivot point based on the event position.
        The point is determined by extracting an area from the renderer's depth buffer
        and choosing the pixel position closest to the event position. This position is then projected
        into world coordinates
        Parameters
        ----------
        event_pos: list[x,y]
            the event position in display coordinates

        Returns
        -------
        True, if a new pivot point has been picked, False otherwise
        """

        self.FindPokedRenderer(event_pos[0], event_pos[1])
        renwin = self.GetInteractor().GetRenderWindow()
        win_size = np.asarray(renwin.GetSize())
        diagonal_len = np.sqrt(np.sum(win_size**2))
        vfa = vtk.vtkFloatArray()
        extent = int(np.ceil(self.tolerance * diagonal_len))
        renwin.GetZbufferData(event_pos[0] - extent, event_pos[1] - extent, event_pos[0]+extent, event_pos[1]+extent, vfa)
        size = extent * 2 + 1
        data = VN.vtk_to_numpy(vfa).reshape(size, size) - 1.0

        nonzero = np.nonzero(data)
        if len(nonzero[0]) > 0:
            distances = (nonzero[0] - extent) ** 2 + (nonzero[1] - extent) ** 2
            min_id = np.argmin(distances)
            nearest_index = (nonzero[0][min_id], nonzero[1][min_id])
            event_pos = (event_pos[0] - extent + nearest_index[0], event_pos[1] - extent + nearest_index[1])
            renderer = self.GetCurrentRenderer()
            world = np.zeros(4)
            self.ComputeDisplayToWorld(renderer, *event_pos, data[nearest_index] + 1.0, world)
            for i in range(3):
                self.pivot[i] = world[i] / world[3]
            return True
        return False

    def _show_pivot_sphere(self):
        """
        Show a sphere at the pivot position
        """
        self._pivot_sphere.SetPosition(self.pivot)

        camera = self.GetCurrentRenderer().GetActiveCamera()
        from_pos = camera.GetPosition()
        vec = np.asarray(self.pivot) - np.asarray(from_pos)
        at_v = _normalize(camera.GetDirectionOfProjection())

        # calculate scale so focus sphere always is the same size on the screen
        s = 0.02 * np.dot(at_v, vec)
        self._pivot_sphere.SetScale(s, s, s)
        self._focus_sphere_renderer = self.GetCurrentRenderer()
        self._focus_sphere_renderer.AddActor(self._pivot_sphere)

        self.GetInteractor().Render()

    def _hide_pivot_sphere(self):
        """
        Hide the sphere if it is visible
        """
        sphere_renderer = getattr(self, '_focus_sphere_renderer', None)
        if sphere_renderer:
            sphere_renderer.RemoveActor(self._pivot_sphere)

    def _left_button_press_event(self, obj, event):
        click_pos = self.GetInteractor().GetEventPosition()
        self.left_down = True
        if not self.right_down:
            self._set_pivot(click_pos)
            self._show_pivot_sphere()
        self.last_pos = click_pos

        super().OnLeftButtonDown()

    def _right_button_press_event(self, obj, event):
        click_pos = self.GetInteractor().GetEventPosition()
        self.right_down = True
        if not self.left_down:
            self._set_pivot(click_pos)
            self._show_pivot_sphere()
        self.last_pos = click_pos
        super().OnMiddleButtonDown()

    def _middle_button_press_event(self, obj, event):
        click_pos = self.GetInteractor().GetEventPosition()
        self._set_pivot(click_pos)
        self.last_pos = click_pos
        super().OnRightButtonDown()

    def _left_button_release_event(self, obj, event):
        self.left_down = False
        if not self.right_down:
            self._hide_pivot_sphere()
        super().OnLeftButtonUp()

    def _right_button_release_event(self, obj, event):
        self.right_down = False
        if not self.left_down:
            self._hide_pivot_sphere()
        super().OnMiddleButtonUp()

    def _middle_button_release_event(self, obj, event):
        super().OnRightButtonUp()

    def _mousewheel_forward_event(self, obj, event):
        event_pos = self.GetInteractor().GetEventPosition()
        self.FindPokedRenderer(event_pos[0], event_pos[1])
        ret = self._set_pivot(event_pos)
        if ret != 0:
            camera = self.GetCurrentRenderer().GetActiveCamera()
            self.StartDolly()
            delta = self.GetMotionFactor() * -0.005 * self.GetMouseWheelMotionFactor()
            if camera.GetParallelProjection():
                camera.SetParallelScale(camera.GetParallelScale / delta)
            else:
                self._dolly_camera(delta)
            self.EndDolly()

    def _mousewheel_backward_event(self, obj, event):
        event_pos = self.GetInteractor().GetEventPosition()
        self.FindPokedRenderer(event_pos[0], event_pos[1])
        ret = self._set_pivot(event_pos)
        if ret != 0:
            camera = self.GetCurrentRenderer().GetActiveCamera()
            self.StartDolly()
            delta = self.GetMotionFactor() * 0.005 * self.GetMouseWheelMotionFactor()
            if camera.GetParallelProjection():
                camera.SetParallelScale(camera.GetParallelScale / delta)
            else:
                self._dolly_camera(delta)
            self.EndDolly()

    def _mouse_move_event(self, obj, event):
        interactor = self.GetInteractor()
        event_pos = interactor.GetEventPosition()
        self.FindPokedRenderer(event_pos[0], event_pos[1])

        if self.GetState() == 1:
            # custom rotation
            self._rotate()
        elif self.GetState() == 2:
            self._pan()
        elif self.GetState() == 4:
            self._dolly()
        else:
            super().OnMouseMove()

        self.last_pos = event_pos
        self.GetInteractor().Render()

    def _normalize_mouse(self, coord):
        size = self.GetInteractor().GetRenderWindow().GetSize()
        return -1.0 + 2.0 * coord[0] / size[0], -1.0 + 2.0 * coord[1] / size[1]

    def _get_right_v_and_up_v(self, pivot, camera):
        cam_pos = np.array(camera.GetPosition())
        pivot = np.asarray(pivot)
        vector_to_pivot = pivot - cam_pos

        if self.translation_plane_normal is None:
            view_plane_normal = _normalize(camera.GetViewPlaneNormal())
        else:
            view_plane_normal = _normalize(self.translation_plane_normal)

        l = -np.dot(vector_to_pivot, view_plane_normal)
        view_angle = camera.GetViewAngle() * np.pi / 180

        size = self.GetInteractor().GetRenderWindow().GetSize()
        scalex = size[0] / size[1] * ((2 * l * np.tan(view_angle / 2)) / 2)
        scaley = ((2 * l * np.tan(view_angle / 2)) / 2)

        view_up = camera.GetViewUp()
        right_v = _normalize(np.cross(view_up, view_plane_normal)) * scalex
        up_v = _normalize(np.cross(view_plane_normal, right_v)) * scaley

        return right_v, up_v

    def _translate_camera(self, translation):
        camera = self.GetCurrentRenderer().GetActiveCamera()
        cam_pos = np.asarray(camera.GetPosition())
        cam_focal = np.asarray(camera.GetFocalPoint())

        cam_pos_new = cam_pos + translation
        cam_focal_new = cam_focal + translation

        camera.SetPosition(cam_pos_new)
        camera.SetFocalPoint(cam_focal_new)

        if self.GetAutoAdjustCameraClippingRange():
            self.GetCurrentRenderer().ResetCameraClippingRange()

        if self.GetInteractor().GetLightFollowCamera():
            self.GetCurrentRenderer().UpdateLightsGeometryToFollowCamera()

    def _rotate_camera(self, point, azimuth, elevation):
        camera = self.GetCurrentRenderer().GetActiveCamera()
        focal_point = camera.GetFocalPoint()
        view_up = camera.GetViewUp()
        position = camera.GetPosition()

        axis = [0, 0, 0]
        axis[0] = -1 * camera.GetViewTransformMatrix().GetElement(0,0)
        axis[1] = -1 * camera.GetViewTransformMatrix().GetElement(0,1)
        axis[2] = -1 * camera.GetViewTransformMatrix().GetElement(0,2)

        transform = vtk.vtkTransform()
        transform.Identity()
        transform.Translate(*point)
        transform.RotateWXYZ(azimuth, view_up)
        transform.RotateWXYZ(elevation, axis)
        transform.Translate(*[-1*x for x in point])

        new_position = transform.TransformPoint(position)
        new_focal_point = transform.TransformPoint(focal_point)

        camera.SetPosition(new_position)
        camera.SetFocalPoint(new_focal_point)

        camera.OrthogonalizeViewUp()

        if self.GetAutoAdjustCameraClippingRange():
            self.GetCurrentRenderer().ResetCameraClippingRange()

        if self.GetInteractor().GetLightFollowCamera():
            self.GetCurrentRenderer().UpdateLightsGeometryToFollowCamera()

    def _pan(self):
        event_pos = self.GetInteractor().GetEventPosition()
        event_pos_normalized = self._normalize_mouse(event_pos)
        last_pos_normalized = self._normalize_mouse(self.last_pos)

        deltax = event_pos_normalized[0] - last_pos_normalized[0]
        deltay = event_pos_normalized[1] - last_pos_normalized[1]

        camera = self.GetCurrentRenderer().GetActiveCamera()

        right_v, up_v = self._get_right_v_and_up_v(self.pivot, camera)
        offset_v = (-deltax * right_v + (-deltay * up_v))

        self._translate_camera(offset_v)

    def _rotate(self):
        interactor = self.GetInteractor()
        dx = interactor.GetEventPosition()[0] - interactor.GetLastEventPosition()[0]
        dy = interactor.GetEventPosition()[1] - interactor.GetLastEventPosition()[1]
        size = self.GetCurrentRenderer().GetRenderWindow().GetSize()

        delta_elevation = -20.0 / size[1]
        delta_azimuth = -20.0 / size[0]

        rxf = dx * delta_azimuth * self.GetMotionFactor()
        ryf = dy * delta_elevation * self.GetMotionFactor()

        self._rotate_camera(self.pivot, rxf, ryf)

    def _dolly_camera(self, delta):
        camera = self.GetCurrentRenderer().GetActiveCamera()
        cam_pos = np.array(camera.GetPosition())

        move = np.asarray(self.pivot) - cam_pos
        offset = move * delta * -4
        self._translate_camera(offset)

        if self.GetAutoAdjustCameraClippingRange():
            self.GetCurrentRenderer().ResetCameraClippingRange()

        if self.GetInteractor().GetLightFollowCamera():
            self.GetCurrentRenderer().UpdateLightsGeometryToFollowCamera()

    def _dolly(self):
        event_pos = self.GetInteractor().GetEventPosition()
        event_pos_normalized = self._normalize_mouse(event_pos)
        last_pos_normalized = self._normalize_mouse(self.last_pos)

        delta = event_pos_normalized[1] - last_pos_normalized[1]
        self._dolly_camera(delta)












