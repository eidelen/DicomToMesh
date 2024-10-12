/****************************************************************************
** Copyright (c) 2017 Adrian Schneider, https://github.com/eidelen
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
*****************************************************************************/

#ifndef DICOM2MESH_H
#define DICOM2MESH_H

#include "volumeVisualizer.h"

#include <string>
#include <vector>
#include <optional>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

class Dicom2Mesh
{
public:
    struct Dicom2MeshParameters
    {
        std::optional<std::string> pathToInputData;
        std::optional<std::vector<std::string>> inputImageFiles;
        std::vector<double> xyzSpacing = {1.0, 1.0, 1.0};
        bool enableCrop = false;
        std::optional<std::string> outputFilePath;
        bool useBinaryExport = false;
        std::optional<double> reductionRate;
        std::optional<unsigned long> polygonLimit;
        std::optional<double> objectSizeRatio;
        bool enableOriginToCenterOfMass = false;
        bool enableSmoothing = false;
        int isoValue = 400; // Hard Tissue
        std::optional<int>  upperIsoValue;

        bool doVisualize = false;
        bool showAsVolume = false;
        std::vector<VolumeRenderingColoringEntry> volumenRenderingColoring;
    };

public:
    Dicom2Mesh(const Dicom2MeshParameters& params);
    ~Dicom2Mesh();

    int doMesh();
    static bool parseCmdLineParameters( const int &argc, const char **argv, Dicom2MeshParameters &param );
    static void showUsageText();
    static void showVersionText();

private:
    std::tuple<bool, vtkSmartPointer<vtkPolyData>, vtkSmartPointer<vtkImageData>> loadInputData();
    std::string getParametersAsString(const Dicom2MeshParameters& params) const;
    static bool parseVolumeRenderingColorEntry( const std::string& text, VolumeRenderingColoringEntry& colorEntry );
    static std::vector<std::string> parseCommaSeparatedStr(const std::string& text);
    static std::string trim(const std::string& str);

private:
    Dicom2MeshParameters m_params;
    vtkSmartPointer<vtkCallbackCommand> m_vtkCallback;
};

#endif // DICOM2MESH_H
