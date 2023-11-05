#include <gtest/gtest.h>
#include "dicom2mesh.h"

#include <filesystem>
#include <cstdio>
#include <limits>

std::ifstream::pos_type filesize(const std::string& filePath)
{
    std::ifstream in(filePath, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

Dicom2Mesh::Dicom2MeshParameters getPresetImageSettings()
{
    Dicom2Mesh::Dicom2MeshParameters settings;
    settings.inputImageFiles = {"lib/test/data/imgset/0.png", "lib/test/data/imgset/1.png",
                                "lib/test/data/imgset/2.png"};
    return settings;
}

TEST(D2M, InvalidMesh)
{
    std::string fname = "testObj.obj";

    Dicom2Mesh::Dicom2MeshParameters settings = getPresetImageSettings();
    settings.outputFilePath = fname;
    settings.isoValue = 400; // to high iso value to create a mesh

    Dicom2Mesh* d2m = new Dicom2Mesh(settings);
    int retCode = d2m->doMesh();
    delete d2m;

    ASSERT_NE(retCode, 0);
    ASSERT_FALSE(std::filesystem::exists(std::filesystem::path(fname)));
}

TEST(D2M, MakeSimpleMesh)
{
    std::vector<std::string> fnames{"testObj.obj", "testPly.ply", "testStl.stl"};

    for( auto fn: fnames )
    {
        Dicom2Mesh::Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fn;
        settings.isoValue = 100;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(fn), 0);
        ASSERT_TRUE(std::filesystem::exists(std::filesystem::path(fn)));
        remove(fn.c_str());
    }
}

TEST(D2M, ImportSimpleMesh)
{
    std::vector<std::string> fnames{"testObj.obj", "testPly.ply", "testStl.stl"};

    for( auto fn: fnames )
    {
        // create file fn
        Dicom2Mesh::Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fn;
        settings.isoValue = 100;
        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(fn), 0);
        ASSERT_TRUE(std::filesystem::exists(std::filesystem::path(fn)));

        // now import it again and safe as, whatever, obj
        std::string exportFilePath = "importtest.obj";
        Dicom2Mesh::Dicom2MeshParameters importSettings;
        importSettings.pathToInputData = fn;
        importSettings.outputFilePath = exportFilePath;
        Dicom2Mesh* d2mImport = new Dicom2Mesh(importSettings);
        retCode = d2mImport->doMesh();
        delete d2mImport;
        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(exportFilePath), 0);
        ASSERT_TRUE(std::filesystem::exists(std::filesystem::path(exportFilePath)));


        remove(fn.c_str());
        remove(exportFilePath.c_str());
    }
}

TEST(D2M, MakeCenterMesh)
{
    std::vector<std::string> fnames{"testObj.obj", "testPly.ply", "testStl.stl"};

    for( auto fn: fnames )
    {
        Dicom2Mesh::Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fn;
        settings.enableOriginToCenterOfMass = true;
        settings.isoValue = 100;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(fn), 0);
        ASSERT_TRUE(std::filesystem::exists(std::filesystem::path(fn)));
        remove(fn.c_str());
    }
}

TEST(D2M, MakeSmoothedMesh)
{
    std::vector<std::string> fnames{"testObj.obj", "testPly.ply", "testStl.stl"};

    for( auto fn: fnames )
    {
        Dicom2Mesh::Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fn;
        settings.enableSmoothing = true;
        settings.isoValue = 100;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_GT(filesize(fn), 0);
        ASSERT_TRUE(std::filesystem::exists(std::filesystem::path(fn)));
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
        Dicom2Mesh::Dicom2MeshParameters settings = getPresetImageSettings();
        settings.outputFilePath = fname;
        settings.isoValue = 100;
        settings.reductionRate = r;

        Dicom2Mesh* d2m = new Dicom2Mesh(settings);
        int retCode = d2m->doMesh();
        delete d2m;

        ASSERT_EQ(retCode, 0);
        ASSERT_TRUE(std::filesystem::exists(std::filesystem::path(fname)));

        // check that file size decreases
        size_t fs = filesize(fname);
        ASSERT_GT(upper, fs);
        upper = fs;

        remove(fname.c_str());
    }
}
