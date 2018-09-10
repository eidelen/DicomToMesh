/****************************************************************************
** Copyright (c) 2017 Adrian Schneider, AOT AG
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

#include <string>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

struct VolumeRenderingColoringEntry
{
    /*
     * The color and transparency
     */
    unsigned char red = 255u;
    unsigned char green = 255u;
    unsigned char blue = 255u;
    unsigned char alpha = 128u;

    /*
     * The voxel value, to which the color is applied
     */
    int voxelValue = 0;
};

struct Dicom2MeshParameters
{
    std::string pathToInputData;
    bool pathToInputAvailable = false;

    std::string outputFilePath = "mesh.stl";
    bool pathToOutputAvailable = false;

    bool enableOriginToCenterOfMass = false;

    double reductionRate = 0.5;
    bool enableMeshReduction = false;

    unsigned long polygonLimit = 100000;
    bool enablePolygonLimitation = false;

    double objectSizeRatio = 0.1;
    bool enableObjectFiltering = false;

    bool enableSmoothing = false;

    bool doVisualize = false;
    bool showAsVolume = false;

    bool enableCrop = false;

    int isoValue = 400; // Hard Tissue

    std::vector<VolumeRenderingColoringEntry> volumenRenderingColoring;
};

class Dicom2Mesh
{
public:
    Dicom2Mesh(const Dicom2MeshParameters& params);
    ~Dicom2Mesh();

    int doMesh();
    static bool parseCmdLineParameters( const int &argc, const char **argv, Dicom2MeshParameters &param );
    static void showUsageText();
    static void showVersionText();

private:
    bool loadInputData( vtkSmartPointer<vtkImageData>& volume, vtkSmartPointer<vtkPolyData>& mesh3d );
    std::string getParametersAsString(const Dicom2MeshParameters& params) const;
    static bool parseVolumeRenderingColorEntry( const std::string& text, VolumeRenderingColoringEntry& colorEntry );

private:
    Dicom2MeshParameters m_params;
    vtkSmartPointer<vtkCallbackCommand> m_vtkCallback;
};

#endif // DICOM2MESH_H
