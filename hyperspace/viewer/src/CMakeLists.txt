set(classes
  vtkOpenGLGridMapper
  vtkPotreeNode
  vtkPotreeLoader
  vtkPotreeMapper
  vtkPotreeMetaData
  vtkPotreeLoaderThread)

vtk_module_add_module(Hyperspace::vtkExtensions
  CLASSES ${classes})

vtk_module_link(Hyperspace::vtkExtensions
        PUBLIC jsoncpp)