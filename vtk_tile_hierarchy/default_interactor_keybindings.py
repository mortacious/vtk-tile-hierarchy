import numpy as np


def _normalize(v):
    v = np.asarray(v)
    norm = np.linalg.norm(v)
    if norm == 0:
        return v
    return v / norm


class DefaultInteractorKeybindsMixin(object):
    def __init__(self):
        self.AddObserver("CharEvent", self.on_char_event)
        self.__key_dispatcher = {
            'f': self._fly_to_point,
            'r': self._reset_camera,
            'p': self._pick,
        }

    def on_char_event(self, obj, event):
        interactor = self.GetInteractor()
        key = interactor.GetKeyCode()
        event_pos = interactor.GetEventPosition()
        self.FindPokedRenderer(event_pos[0], event_pos[1])

        try:
            self.__key_dispatcher[key]()
        except KeyError:
            pass

    def _fly_to_point(self):
        interactor = self.GetInteractor()
        event_pos = interactor.GetEventPosition()
        self._set_pivot(event_pos)
        self._show_pivot_sphere()

        interactor.SetDolly(1.0)
        interactor.FlyTo(self.GetCurrentRenderer(), self.pivot)
        self._hide_pivot_sphere()

    def _reset_camera(self):
        renderer = self.GetCurrentRenderer()
        if renderer:
            renderer.ResetCamera()
        self.GetInteractor().Render()

    def _pick(self):
        interactor = self.GetInteractor()
        renderer = self.GetCurrentRenderer()
        if renderer:
            interactor.StartPickCallback()
            picker = interactor.GetPicker()
            if picker:
                event_pos = interactor.GetEventPosition()
                picker.Pick(event_pos[0], event_pos[1], 0.0, renderer)
            interactor.EndPickCallback()


