#include <gtest/gtest.h>
#include "dicom2mesh.h"

TEST(ArgumentParser, InputPathAndDefault)
{
    int nInput = 2;
    const char* input[2] = { "-i", "inputDir"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.pathToInputData.has_value());
    ASSERT_STREQ(parsedInput.pathToInputData.value().c_str(),"inputDir");

    ASSERT_FALSE(parsedInput.reductionRate);
    ASSERT_FALSE(parsedInput.enableCrop);
    ASSERT_FALSE(parsedInput.polygonLimit);
    ASSERT_FALSE(parsedInput.enableSmoothing);
    ASSERT_FALSE(parsedInput.objectSizeRatio.has_value());
    ASSERT_FALSE(parsedInput.enableOriginToCenterOfMass);

    ASSERT_FALSE(parsedInput.outputFilePath.has_value());
}

TEST(ArgumentParser, OutputPath)
{
    int nInput = 4;
    const char *input[4] = {"-i", "inputDir", "-o", "output.obj"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.outputFilePath.has_value());
    ASSERT_STREQ(parsedInput.outputFilePath.value().c_str(), "output.obj");
}

TEST(ArgumentParser, Threshold)
{
    int nInput = 4;
    const char *input[4] = {"-i", "inputDir", "-t", "405"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_EQ(parsedInput.isoValue, 405);
}

TEST(ArgumentParser, ThresholdRange)
{
    int nInput = 6;
    const char *input[6] = {"-i", "inputDir", "-t", "405", "-tu", "501"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_EQ(parsedInput.isoValue, 405);
    ASSERT_EQ(parsedInput.upperIsoValue.value(), 501);
    ASSERT_TRUE(parsedInput.upperIsoValue.has_value());
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

    ASSERT_TRUE(parsedInput.reductionRate);
    ASSERT_FLOAT_EQ(parsedInput.reductionRate.value(), 0.43);
}

TEST(ArgumentParser, SmoothingAndFilter)
{
    int nInput = 5;
    const char *input[5] = {"-i", "inputDir", "-s", "-e", "0.1234"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.enableSmoothing);

    ASSERT_TRUE(parsedInput.objectSizeRatio.has_value());
    ASSERT_FLOAT_EQ(parsedInput.objectSizeRatio.value(), 0.1234);
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
    int nInput = 17;
    const char *input[17] = {"-i", "inputDir", "-vo", "( ", "1,", "2  ,", " 3 ,", "4", " ,-5)", "(6 , ", "7,", "+8,", "9 ,", "10 ", ")", "(252,253,254,255,0)", "(100,101,102,103,104)" };

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.doVisualize);
    ASSERT_TRUE(parsedInput.showAsVolume);

    ASSERT_EQ(4,parsedInput.volumenRenderingColoring.size());

    ASSERT_EQ(1, parsedInput.volumenRenderingColoring.at(0).m_red);
    ASSERT_EQ(2, parsedInput.volumenRenderingColoring.at(0).m_green);
    ASSERT_EQ(3, parsedInput.volumenRenderingColoring.at(0).m_blue);
    ASSERT_EQ(4, parsedInput.volumenRenderingColoring.at(0).m_alpha);
    ASSERT_EQ(-5, parsedInput.volumenRenderingColoring.at(0).m_voxelValue);

    ASSERT_EQ(6, parsedInput.volumenRenderingColoring.at(1).m_red);
    ASSERT_EQ(7, parsedInput.volumenRenderingColoring.at(1).m_green);
    ASSERT_EQ(8, parsedInput.volumenRenderingColoring.at(1).m_blue);
    ASSERT_EQ(9, parsedInput.volumenRenderingColoring.at(1).m_alpha);
    ASSERT_EQ(10, parsedInput.volumenRenderingColoring.at(1).m_voxelValue);

    ASSERT_EQ(252, parsedInput.volumenRenderingColoring.at(2).m_red);
    ASSERT_EQ(253, parsedInput.volumenRenderingColoring.at(2).m_green);
    ASSERT_EQ(254, parsedInput.volumenRenderingColoring.at(2).m_blue);
    ASSERT_EQ(255, parsedInput.volumenRenderingColoring.at(2).m_alpha);
    ASSERT_EQ(0, parsedInput.volumenRenderingColoring.at(2).m_voxelValue);

    ASSERT_EQ(100, parsedInput.volumenRenderingColoring.at(3).m_red);
    ASSERT_EQ(101, parsedInput.volumenRenderingColoring.at(3).m_green);
    ASSERT_EQ(102, parsedInput.volumenRenderingColoring.at(3).m_blue);
    ASSERT_EQ(103, parsedInput.volumenRenderingColoring.at(3).m_alpha);
    ASSERT_EQ(104, parsedInput.volumenRenderingColoring.at(3).m_voxelValue);
}

TEST(ArgumentParser, VolVisualizationInvalidColor)
{
    int nInput = 9;
    const char *input[9] = {"-i", "inputDir", "-vo", "( ", "257,", "2  ,", " 3 ,", "4", " ,-5)"};

    Dicom2MeshParameters parsedInput;
    ASSERT_FALSE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));
}

TEST(ArgumentParser, ManyParamsEnabled)
{
    int nInput = 12;
    const char *input[12] = {"-i", "inputDir", "-c", "-z", "-v", "-s", "-e", "0.1234", "-r", "0.43","-o", "output.obj"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.pathToInputData.has_value());
    ASSERT_STREQ(parsedInput.pathToInputData.value().c_str(), "inputDir");

    ASSERT_TRUE(parsedInput.doVisualize);
    ASSERT_FALSE(parsedInput.showAsVolume);

    ASSERT_TRUE(parsedInput.enableOriginToCenterOfMass);
    ASSERT_TRUE(parsedInput.enableCrop);

    ASSERT_TRUE(parsedInput.enableSmoothing);

    ASSERT_TRUE(parsedInput.objectSizeRatio.has_value());
    ASSERT_FLOAT_EQ(parsedInput.objectSizeRatio.value(), 0.1234);

    ASSERT_TRUE(parsedInput.reductionRate);
    ASSERT_FLOAT_EQ(parsedInput.reductionRate.value(), 0.43);

    ASSERT_TRUE(parsedInput.outputFilePath.has_value());
    ASSERT_STREQ(parsedInput.outputFilePath.value().c_str(), "output.obj");
}

TEST(ArgumentParser, ManyParamsDisabled)
{
    int nInput = 2;
    const char *input[2] = {"-i", "inputDir"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(nInput, input, parsedInput));

    ASSERT_TRUE(parsedInput.pathToInputData.has_value());
    ASSERT_STREQ(parsedInput.pathToInputData.value().c_str(),"inputDir");

    ASSERT_FALSE(parsedInput.doVisualize);
    ASSERT_FALSE(parsedInput.showAsVolume);

    ASSERT_FALSE(parsedInput.enableOriginToCenterOfMass);
    ASSERT_FALSE(parsedInput.enableCrop);

    ASSERT_FALSE(parsedInput.enableSmoothing);

    ASSERT_FALSE(parsedInput.objectSizeRatio.has_value());

    ASSERT_FALSE(parsedInput.reductionRate);

    ASSERT_FALSE(parsedInput.outputFilePath.has_value());
}

TEST(ArgumentParser, ParsePngInput)
{
    const char *input[4] = {"-ipng", "[abcd/efgh.png", ", hi/jkl.png", "]"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(4, input, parsedInput));
    ASSERT_TRUE(parsedInput.inputImageFiles);
    ASSERT_EQ(parsedInput.inputImageFiles.value().size(),2);

    ASSERT_STREQ( "abcd/efgh.png", parsedInput.inputImageFiles.value()[0].c_str() );
    ASSERT_STREQ( "hi/jkl.png", parsedInput.inputImageFiles.value()[1].c_str() );
}

TEST(ArgumentParser, ParseSpacing)
{
    const char *input[6] = {"-i", "inputDir", "-sxyz", "1.5", "2.0 ",  " 3.0 "};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(6, input, parsedInput));

    ASSERT_FLOAT_EQ(parsedInput.x_spacing, 1.5);
    ASSERT_FLOAT_EQ(parsedInput.y_spacing, 2.0);
    ASSERT_FLOAT_EQ(parsedInput.z_spacing, 3.0);
}

TEST(ArgumentParser, ParseBinaryExportOn)
{
    const char *input[6] = {"-i", "inputDir", "-b"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(3, input, parsedInput));

    ASSERT_TRUE(parsedInput.useBinaryExport);
}

TEST(ArgumentParser, ParseBinaryExportOff)
{
    const char *input[6] = {"-i", "inputDir"};

    Dicom2MeshParameters parsedInput;
    ASSERT_TRUE(Dicom2Mesh::parseCmdLineParameters(2, input, parsedInput));

    ASSERT_FALSE(parsedInput.useBinaryExport);
}
