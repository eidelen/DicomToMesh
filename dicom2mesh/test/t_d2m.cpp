#include <gtest/gtest.h>
#include "dicom2mesh.h"

Dicom2MeshParameters getPresetImageSettings()
{
    Dicom2MeshParameters settings;
    settings.inputImageFiles = {"lib/test/data/imgset/0.png", "lib/test/data/imgset/1.png",
                                "lib/test/data/imgset/2.png"};
    settings.inputAsPngFileList = true;
    return settings;
}

TEST(D2M, InvalidMesh)
{
    Dicom2MeshParameters settings = getPresetImageSettings();
    settings.outputFilePath = "testObj.obj";
    settings.isoValue = 400; // to high iso value to create a mesh

    Dicom2Mesh* d2m = new Dicom2Mesh(settings);
    int retCode = d2m->doMesh();
    delete d2m;

    ASSERT_NE(retCode, 0);
}

TEST(D2M, MakeRawObj)
{
    Dicom2MeshParameters settings = getPresetImageSettings();
    settings.outputFilePath = "testObj.obj";
    settings.isoValue = 100;

    Dicom2Mesh* d2m = new Dicom2Mesh(settings);
    int retCode = d2m->doMesh();
    delete d2m;

    ASSERT_EQ(retCode, 0);
}

