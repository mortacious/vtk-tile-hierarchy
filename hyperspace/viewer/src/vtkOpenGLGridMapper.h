/**
 * @class   vtkOpenGLGridMapper
 * @brief   A mapper which display an infinite plane
 *
 */

#pragma once

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkSmartPointer.h>
#include "vtkHyperspaceExtensionsModule.h"

class VTKHYPERSPACEEXTENSIONS_EXPORT vtkOpenGLGridMapper : public vtkOpenGLPolyDataMapper
{
public:
  static vtkOpenGLGridMapper* New();
  vtkTypeMacro(vtkOpenGLGridMapper, vtkOpenGLPolyDataMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
  /**
   * Set/Get the distance where the grid disappear.
   */
  vtkSetMacro(FadeDistance, double);
  vtkGetMacro(FadeDistance, double);
  //@}

  //@{
  /**
   * Set/Get the size of a square on the grid.
   */
  vtkSetMacro(UnitSquare, double);
  vtkGetMacro(UnitSquare, double);
  //@}

  //@{
  /**
   * Set/Get the up vector index (X, Y, Z axis respectively).
   */
  vtkSetClampMacro(UpIndex, int, 0, 2);
  vtkGetMacro(UpIndex, int);
  //@}

  double* GetBounds() override;

protected:
  vtkOpenGLGridMapper();
  ~vtkOpenGLGridMapper() = default;

  void ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;

  void SetMapperShaderParameters(
    vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor) override;

  void BuildBufferObjects(vtkRenderer* ren, vtkActor* act) override;

  void RenderPiece(vtkRenderer* ren, vtkActor* actor) override;

  bool GetNeedToRebuildShaders(vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* act) override;

  double FadeDistance = 10.0;
  double UnitSquare = 1.0;
  int UpIndex = 1;

private:
  vtkOpenGLGridMapper(const vtkOpenGLGridMapper&) = delete;
  void operator=(const vtkOpenGLGridMapper&) = delete;
};