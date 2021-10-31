set(classes
  vtkOpenGLGridMapper
  vtkPotreeMapper
  vtkPotreeLoader
  vtkPotreeMetaData)

vtk_module_add_module(Hyperspace::vtkExtensions
  CLASSES ${classes})