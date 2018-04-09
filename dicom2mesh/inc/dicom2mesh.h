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

#include <string>

#ifndef DICOM2MESH_H
#define DICOM2MESH_H

using namespace std;

struct Dicom2MeshParameters
{
    string pathToInputData;
    bool pathToDicomSet = false;
    bool enabledExportMeshFile = false;
    bool setOriginToCenterOfMass = false;
    bool enableMeshReduction = false;
    bool enablePolygonLimitation = false;
    bool extracOnlyBigObjects = false;
    bool enableSmoothing = false;
    bool showIn3DView = false;
    bool enableCrop = false;
    char pad[3];
    int isoValue = 400; // Hard Tissue
    unsigned long polygonLimit = 100000;
    double nbrVerticesRatio = 0.1;
    double reductionRate = 0.5;
    string outputFilePath = "mesh.stl";
};

#endif // DICOM2MESH_H