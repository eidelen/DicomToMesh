#include <gtest/gtest.h>
#include "vtkMeshRoutines.h"

TEST(Mesh, ConstructDestruct)
{
    ASSERT_NO_THROW
    (
        VTKMeshRoutines* vM = new VTKMeshRoutines();
        delete vM;
    );
}

TEST(Mesh, OpenOBJ)
{
    VTKMeshRoutines* vM = new VTKMeshRoutines();

    vtkSmartPointer<vtkPolyData> mesh = vM->importObjFile( "lib/test/data/torus.obj" );

    vtkIdType numberOfVertices = mesh->GetPoints()->GetNumberOfPoints();
    vtkIdType numberOfFaces = mesh->GetNumberOfCells();

    ASSERT_EQ( numberOfVertices, 2304 );
    ASSERT_EQ( numberOfFaces, 576 );

    delete vM;

}
// to be extended ...