set(classes
  vtkOpenGLGridMapper
  vtkPotreeMapper
  vtkPotreeLoader)

vtk_module_add_module(Hyperspace::vtkExtensions
  CLASSES ${classes})