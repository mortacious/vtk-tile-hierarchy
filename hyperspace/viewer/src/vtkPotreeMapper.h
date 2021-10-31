//
// Created by mortacious on 10/29/21.
//

#pragma once
#include <vtkSmartPointer.h>
#include <vtkMapper.h>
#include "vtkHyperspaceExtensionsModule.h" // For export macro

class vtkRenderer;
class vtkActor;
class vtkPotreeMapperInternals;

class VTKHYPERSPACEEXTENSIONS_EXPORT vtkPotreeMapper : public vtkMapper
{
public:
    static vtkPotreeMapper* New();
    vtkTypeMacro(vtkPotreeMapper, vtkMapper);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    /**
     * Standard method for rendering a mapper. This method will be
     * called by the actor.
     */
    void Render(vtkRenderer* ren, vtkActor* a) override;

    ///@{
    /**
     * Standard vtkProp method to get 3D bounds of a 3D prop
     */
    double* GetBounds() VTK_SIZEHINT(6) override;
    void GetBounds(double bounds[6]) override { this->Superclass::GetBounds(bounds); }

    //@{
    /**
     * Set/Get the mapper used to render the pieces.
     */
    vtkSetMacro(PieceMapper, vtkMapper*);
    vtkGetMacro(PieceMapper, vtkMapper*);
protected:
    vtkPotreeMapper();
    ~vtkPotreeMapper() = default;

    /**
     * Need to loop over the hierarchy to compute bounds
     */
    void ComputeBounds();

    virtual vtkMapper* MakeMapper();

    /**
     * Time stamp for computation of bounds.
     */
    vtkTimeStamp BoundsMTime;

    /**
     * These are the internal polydata mapper that do the
     * rendering. We save then so that they can keep their
     * display lists.
     */
    vtkPotreeMapperInternals* Internal;

    /**
     * Time stamp for when we need to update the
     * internal mappers
     */
    vtkTimeStamp InternalMappersBuildTime;

    vtkMapper* PieceMapper;
private:
    vtkPotreeMapper(const vtkPotreeMapper&) = delete;
    void operator=(const vtkPotreeMapper&) = delete;
};