set(classes
  vtkOpenGLGridMapper
  vtkPotreeMapper
  vtkPotreeLoader
  vtkPotreeMetaData
  vtkPotreeLoaderThread)

vtk_module_add_module(Hyperspace::vtkExtensions
  CLASSES ${classes})