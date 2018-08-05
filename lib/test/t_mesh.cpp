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

    ASSERT_EQ( numberOfVertices, 3456 );
    ASSERT_EQ( numberOfFaces, 1152 );

    delete vM;
}

TEST(Mesh, ReduceMesh)
{
    VTKMeshRoutines* vM = new VTKMeshRoutines();

    vtkSmartPointer<vtkPolyData> mesh = vM->importObjFile( "lib/test/data/torus.obj" );
    vtkIdType numberOfVertices = mesh->GetPoints()->GetNumberOfPoints();

    for(int k = 0; k < 10; k++ )
    {
        vM->meshReduction(mesh, 0.1);
        vtkIdType newNumberOfVertices = mesh->GetPoints()->GetNumberOfPoints();
        ASSERT_LT( newNumberOfVertices, numberOfVertices );
        numberOfVertices = newNumberOfVertices;
    }

    delete vM;
}

TEST(Mesh, CenterObject)
{
    VTKMeshRoutines* vM = new VTKMeshRoutines();

    vtkSmartPointer<vtkPolyData> mesh = vM->importObjFile( "lib/test/data/torus.obj" );
    VTKMeshRoutines::Vector3 trans = vM->moveMeshToCOSCenter(mesh);

    ASSERT_NEAR(trans.x, 1.0, 0.1);
    ASSERT_NEAR(trans.y, 3.0, 0.1);
    ASSERT_NEAR(trans.z, -2.0, 0.1);

    delete vM;
}

// to be extended ...