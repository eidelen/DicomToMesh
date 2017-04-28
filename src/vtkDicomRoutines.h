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

#ifndef _vtkDicomRoutines_H_
#define _vtkDicomRoutines_H_

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>
#include <vtkCallbackCommand.h>
#include <string>

class VTKDicomRoutines
{

public:

    /**
     * Loads the DICOM images within a directory.
     * @param pathToDicom Path to the DICOM directory.
     * @param progressCallback Progress callback function.
     * @return DICOM image data.
     */
    static vtkImageData* loadDicomImage( const std::string& pathToDicom, vtkSmartPointer<vtkCallbackCommand> progressCallback );

    /**
     * Creates a mesh from out DICOM raw data.
     * @param imageData DICOM image data.
     * @param threshold Threshold for surface segmentation.
     * @param progressCallback Progress callback function.
     * @return Resulting 3D mesh.
     */
    static vtkPolyData* dicomToMesh( vtkSmartPointer<vtkImageData> imageData, const int& threshold, vtkSmartPointer<vtkCallbackCommand> progressCallback );

};

#endif // _vtkDicomRoutines_H_
