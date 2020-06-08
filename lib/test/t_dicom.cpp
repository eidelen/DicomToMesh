#include <gtest/gtest.h>
#include "dicomRoutines.h"

TEST(Dicom, LoadFromInexistentPng)
{
    VTKDicomRoutines* dr = new VTKDicomRoutines();

    std::vector<std::string> paths = {"lib/test/data/nonexistent/0.png", "lib/test/data/nonexistent/1.png"};

    vtkSmartPointer<vtkImageData> imgData = dr->loadPngImages(paths, 1.0, 1.0, 1.0);
    ASSERT_TRUE(imgData.Get() == nullptr);

    delete dr;
}

TEST(Dicom, LoadFromPngs)
{
    VTKDicomRoutines* dr = new VTKDicomRoutines();

    std::vector<std::string> paths = {"lib/test/data/imgset/0.png", "lib/test/data/imgset/1.png",
                                      "lib/test/data/imgset/2.png"};

    vtkSmartPointer<vtkImageData> imgData = dr->loadPngImages(paths, 1.0, 1.5, 2.0);
    ASSERT_FALSE(imgData.Get() == nullptr);

    int* dims = imgData->GetDimensions();
    ASSERT_EQ(dims[0], 256);
    ASSERT_EQ(dims[1], 256);
    ASSERT_EQ(dims[2], 3);

    double* sps = imgData->GetSpacing();
    ASSERT_NEAR(sps[0], 1.0, 0.001);
    ASSERT_NEAR(sps[1], 1.5, 0.001);
    ASSERT_NEAR(sps[2], 2.0, 0.001);

    delete dr;
}

TEST(Dicom, CreateFromPngs)
{
    VTKDicomRoutines* dr = new VTKDicomRoutines();

    // those images are black (0) and objects are drawn with gray (200)
    std::vector<std::string> paths = {"lib/test/data/imgset/0.png", "lib/test/data/imgset/1.png",
                                      "lib/test/data/imgset/2.png"};

    vtkSmartPointer<vtkImageData> imgData = dr->loadPngImages(paths, 1.0, 1.5, 2.0);
    ASSERT_FALSE(imgData.Get() == nullptr);

    // valid range
    vtkSmartPointer<vtkPolyData> meshA = dr->dicomToMesh(imgData, 10, false, 255);
    vtkIdType nbrFacesA = meshA->GetNumberOfCells();
    ASSERT_GT( nbrFacesA, 0 );

    // specific exact range
    vtkSmartPointer<vtkPolyData> meshB = dr->dicomToMesh(imgData, 199, true, 201);
    vtkIdType nbrFacesB = meshB->GetNumberOfCells();
    ASSERT_EQ( nbrFacesA, nbrFacesB );

    // out of range
    vtkSmartPointer<vtkPolyData> meshC = dr->dicomToMesh(imgData, 300, false, 1000);
    vtkIdType nbrFacesC = meshC->GetNumberOfCells();
    ASSERT_EQ( nbrFacesC, 0 );

    delete dr;
}
