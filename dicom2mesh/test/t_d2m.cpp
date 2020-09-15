#include <gtest/gtest.h>
#include "dicom2mesh.h"

#include <fstream>
#include <cstdio>
#include <limits>

bool fexists(const std::string& filePath)
{
    std::ifstream f(filePath.c_str());
    return f.good();
}

std::ifstream::pos_type filesize(const std::string& filePath)
{
    std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

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
    std::string fname = "testObj.obj";

    Dicom2MeshParameters settings = getPresetImageSettings();
    settings.outputFilePath = fname;
    settings.pathToOutputAvailable = true;
    settings.isoValue = 400; // to high iso value to create a mesh

    Dicom2Mesh* d2m = new Dicom2Mesh(settings);
    int retCode = d2m->doMesh();
    delete d2m;

    ASSERT_NE(retCode, 0);
    ASSERT_FALSE(fexists(fname));
}

TEST(D2M, MakeSimpleMesh)
{
    std::vector<std::string> fnames{"testObj.obj", "testPly.ply", "testStl.stl"};

    for( auto fn: fnames )
    {
        Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fn;
        settings.pathToOutputAvailable = true;
        settings.isoValue = 100;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(fn), 0);
        ASSERT_TRUE(fexists(fn));
        remove(fn.c_str());
    }
}

TEST(D2M, MakeCenterMesh)
{
    std::vector<std::string> fnames{"testObj.obj", "testPly.ply", "testStl.stl"};

    for( auto fn: fnames )
    {
        Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fn;
        settings.pathToOutputAvailable = true;
        settings.enableOriginToCenterOfMass = true;
        settings.isoValue = 100;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(fn), 0);
        ASSERT_TRUE(fexists(fn));
        remove(fn.c_str());
    }
}

TEST(D2M, MakeSmoothedMesh)
{
    std::vector<std::string> fnames{"testObj.obj", "testPly.ply", "testStl.stl"};

    for( auto fn: fnames )
    {
        Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fn;
        settings.pathToOutputAvailable = true;
        settings.enableSmoothing = true;
        settings.isoValue = 100;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(fn), 0);
        ASSERT_TRUE(fexists(fn));
        remove(fn.c_str());
    }
}

TEST(D2M, TestReduce)
{
    std::string fname = "testObj.obj";
    std::vector<double> reduce = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9 };

    size_t upper = std::numeric_limits<size_t>::max();

    for( double r: reduce )
    {
        Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fname;
        settings.pathToOutputAvailable = true;
        settings.isoValue = 100;
        settings.enableMeshReduction = true;
        settings.reductionRate = r;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_TRUE(fexists(fname));

        // check that file size decreases
        size_t fs = filesize(fname);
        ASSERT_GT(upper, fs);
        upper = fs;

        remove(fname.c_str());
    }
}
