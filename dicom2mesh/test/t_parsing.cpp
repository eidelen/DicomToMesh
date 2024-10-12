#include <gtest/gtest.h>
#include "dicom2mesh.h"

TEST(ArgumentParser, InputPathAndDefault)
{
    constexpr int nInput = 2;
    const char* input[nInput] = { "-i", "inputDir"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.pathToInputData.has_value());
    ASSERT_STREQ(parsedInput.pathToInputData.value().c_str(),"inputDir");

    ASSERT_FALSE(parsedInput.reductionRate.has_value());
    ASSERT_FALSE(parsedInput.enableCrop);
    ASSERT_FALSE(parsedInput.polygonLimit);
    ASSERT_FALSE(parsedInput.enableSmoothing);
    ASSERT_FALSE(parsedInput.objectSizeRatio.has_value());
    ASSERT_FALSE(parsedInput.enableOriginToCenterOfMass);

    ASSERT_FALSE(parsedInput.outputFilePath.has_value());
}

TEST(ArgumentParser, OutputPath)
{
    constexpr int nInput = 4;
    const char *input[nInput] = {"-i", "inputDir", "-o", "output.obj"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.outputFilePath.has_value());
    ASSERT_STREQ(parsedInput.outputFilePath.value().c_str(), "output.obj");
}

TEST(ArgumentParser, Threshold)
{
    constexpr int nInput = 4;
    const char *input[nInput] = {"-i", "inputDir", "-t", "405"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_EQ(parsedInput.isoValue, 405);
}

TEST(ArgumentParser, ThresholdRange)
{
    constexpr int nInput = 6;
    const char *input[nInput] = {"-i", "inputDir", "-t", "405", "-tu", "501"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_EQ(parsedInput.isoValue, 405);
    ASSERT_TRUE(parsedInput.upperIsoValue.has_value());
    ASSERT_EQ(parsedInput.upperIsoValue.value(), 501);
}

TEST(ArgumentParser, ThresholdNegative)
{
    constexpr int nInput = 4;
    const char *input[nInput] = {"-i", "inputDir", "-t", "-24"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_EQ(parsedInput.isoValue, -24);
}

TEST(ArgumentParser, Reduction)
{
    constexpr int nInput = 4;
    const char *input[nInput] = {"-i", "inputDir", "-r", "0.43"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.reductionRate.has_value());
    ASSERT_FLOAT_EQ(parsedInput.reductionRate.value(), 0.43);
}

TEST(ArgumentParser, SmoothingAndFilter)
{
    constexpr int nInput = 5;
    const char *input[nInput] = {"-i", "inputDir", "-s", "-e", "0.1234"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.enableSmoothing);

    ASSERT_TRUE(parsedInput.objectSizeRatio.has_value());
    ASSERT_FLOAT_EQ(parsedInput.objectSizeRatio.value(), 0.1234);
}

TEST(ArgumentParser, CenterAndCrop)
{
    constexpr int nInput = 4;
    const char *input[nInput] = {"-i", "inputDir", "-c", "-z"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.enableOriginToCenterOfMass);
    ASSERT_TRUE(parsedInput.enableCrop);
}

TEST(ArgumentParser, Visualization)
{
    constexpr int nInput = 3;
    const char *input[nInput] = {"-i", "inputDir", "-v"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.doVisualize);
    ASSERT_FALSE(parsedInput.showAsVolume);
}


TEST(ArgumentParser, VolVisualization)
{
    constexpr int nInput = 17;
    const char *input[nInput] = {"-i", "inputDir", "-vo", "( ", "1,", "2  ,", " 3 ,", "4", " ,-5)", "(6 , ", "7,", "+8,", "9 ,", "10 ", ")", "(252,253,254,255,0)", "(100,101,102,103,104)" };

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

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
    constexpr int nInput = 9;
    const char *input[nInput] = {"-i", "inputDir", "-vo", "( ", "257,", "2  ,", " 3 ,", "4", " ,-5)"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_FALSE(okPars);
}

TEST(ArgumentParser, ManyParamsEnabled)
{
    constexpr int nInput = 12;
    const char *input[nInput] = {"-i", "inputDir", "-c", "-z", "-v", "-s", "-e", "0.1234", "-r", "0.43","-o", "output.obj"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.pathToInputData.has_value());
    ASSERT_STREQ(parsedInput.pathToInputData.value().c_str(), "inputDir");

    ASSERT_TRUE(parsedInput.doVisualize);
    ASSERT_FALSE(parsedInput.showAsVolume);

    ASSERT_TRUE(parsedInput.enableOriginToCenterOfMass);
    ASSERT_TRUE(parsedInput.enableCrop);

    ASSERT_TRUE(parsedInput.enableSmoothing);

    ASSERT_TRUE(parsedInput.objectSizeRatio.has_value());
    ASSERT_FLOAT_EQ(parsedInput.objectSizeRatio.value(), 0.1234);

    ASSERT_TRUE(parsedInput.reductionRate.has_value());
    ASSERT_FLOAT_EQ(parsedInput.reductionRate.value(), 0.43);

    ASSERT_TRUE(parsedInput.outputFilePath.has_value());
    ASSERT_STREQ(parsedInput.outputFilePath.value().c_str(), "output.obj");
}

TEST(ArgumentParser, ManyParamsDisabled)
{
    constexpr int nInput = 2;
    const char *input[nInput] = {"-i", "inputDir"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.pathToInputData.has_value());
    ASSERT_STREQ(parsedInput.pathToInputData.value().c_str(),"inputDir");

    ASSERT_FALSE(parsedInput.doVisualize);
    ASSERT_FALSE(parsedInput.showAsVolume);

    ASSERT_FALSE(parsedInput.enableOriginToCenterOfMass);
    ASSERT_FALSE(parsedInput.enableCrop);

    ASSERT_FALSE(parsedInput.enableSmoothing);

    ASSERT_FALSE(parsedInput.objectSizeRatio.has_value());

    ASSERT_FALSE(parsedInput.reductionRate.has_value());

    ASSERT_FALSE(parsedInput.outputFilePath.has_value());
}

TEST(ArgumentParser, ParsePngInput)
{
    const char *input[4] = {"-ipng", "[abcd/efgh.png", ", hi/jkl.png", "]"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(4, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.inputImageFiles.has_value());
    ASSERT_EQ(parsedInput.inputImageFiles.value().size(),2);

    ASSERT_STREQ( "abcd/efgh.png", parsedInput.inputImageFiles.value()[0].c_str() );
    ASSERT_STREQ( "hi/jkl.png", parsedInput.inputImageFiles.value()[1].c_str() );
}

TEST(ArgumentParser, ParseSpacing)
{
    constexpr int nInput = 6;
    const char *input[nInput] = {"-i", "inputDir", "-sxyz", "1.5", "2.0 ",  " 3.0 "};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_FLOAT_EQ(parsedInput.xyzSpacing[0], 1.5);
    ASSERT_FLOAT_EQ(parsedInput.xyzSpacing[1], 2.0);
    ASSERT_FLOAT_EQ(parsedInput.xyzSpacing[2], 3.0);
}

TEST(ArgumentParser, ParseBinaryExportOn)
{
    constexpr int nInput = 3;
    const char *input[nInput] = {"-i", "inputDir", "-b"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.useBinaryExport);
}

TEST(ArgumentParser, ParseBinaryExportOff)
{
    constexpr int nInput = 2;
    const char *input[nInput] = {"-i", "inputDir"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_FALSE(parsedInput.useBinaryExport);
}

TEST(ArgumentParser, PolygonLimitEnable)
{
    constexpr int nInput = 4;
    const char *input[nInput] = {"-i", "inputDir", "-p", "12345"};

    auto[okPars, parsedInput] = Dicom2Mesh::parseCmdLineParameters(nInput, input);

    ASSERT_TRUE(okPars);

    ASSERT_TRUE(parsedInput.polygonLimit.has_value());
    ASSERT_EQ(parsedInput.polygonLimit, 12345);
}
