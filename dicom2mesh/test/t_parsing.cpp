#include <gtest/gtest.h>
#include "dicom2mesh.h"

TEST(ArgumentParser, InputPathAndDefault)
{
    int nInput = 2;
    const char* input[2] = { "-i", "inputDir"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.pathToInputAvailable);
    ASSERT_STREQ(parsedInput.pathToInputData.c_str(),"inputDir");

    ASSERT_FALSE(parsedInput.enableMeshReduction);
    ASSERT_FALSE(parsedInput.enableCrop);
    ASSERT_FALSE(parsedInput.enablePolygonLimitation);
    ASSERT_FALSE(parsedInput.enableSmoothing);
    ASSERT_FALSE(parsedInput.enableObjectFiltering);
    ASSERT_FALSE(parsedInput.enableOriginToCenterOfMass);

    ASSERT_FALSE(parsedInput.pathToOutputAvailable);
}

TEST(ArgumentParser, OutputPath)
{
    int nInput = 4;
    const char *input[4] = {"-i", "inputDir", "-o", "output.obj"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.pathToOutputAvailable);
    ASSERT_STREQ(parsedInput.outputFilePath.c_str(), "output.obj");
}

TEST(ArgumentParser, Threshold)
{
    int nInput = 4;
    const char *input[4] = {"-i", "inputDir", "-t", "405"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_EQ(parsedInput.isoValue, 405);
}

TEST(ArgumentParser, ThresholdNegative)
{
    int nInput = 4;
    const char *input[4] = {"-i", "inputDir", "-t", "-24"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_EQ(parsedInput.isoValue, -24);
}

TEST(ArgumentParser, Reduction)
{
    int nInput = 4;
    const char *input[4] = {"-i", "inputDir", "-r", "0.43"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.enableMeshReduction);
    ASSERT_FLOAT_EQ(parsedInput.reductionRate, 0.43);
}

TEST(ArgumentParser, SmoothingAndFilter)
{
    int nInput = 5;
    const char *input[5] = {"-i", "inputDir", "-s", "-e", "0.1234"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.enableSmoothing);

    ASSERT_TRUE(parsedInput.enableObjectFiltering);
    ASSERT_FLOAT_EQ(parsedInput.objectSizeRatio, 0.1234);
}

TEST(ArgumentParser, CenterAndCrop)
{
    int nInput = 4;
    const char *input[4] = {"-i", "inputDir", "-c", "-z"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.enableOriginToCenterOfMass);
    ASSERT_TRUE(parsedInput.enableCrop);
}


TEST(ArgumentParser, Visualization)
{
    int nInput = 3;
    const char *input[3] = {"-i", "inputDir", "-v"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.doVisualize);
    ASSERT_FALSE(parsedInput.showAsVolume);
}


TEST(ArgumentParser, VolVisualization)
{
    int nInput = 3;
    const char *input[3] = {"-i", "inputDir", "-vo"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.doVisualize);
    ASSERT_TRUE(parsedInput.showAsVolume);
}
