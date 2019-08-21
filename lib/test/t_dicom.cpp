#include <gtest/gtest.h>
#include "vtkDicomRoutines.h"

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
