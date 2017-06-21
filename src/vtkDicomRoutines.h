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

    VTKDicomRoutines();
    ~VTKDicomRoutines();

    /**
     * Sets the progress callback function.
     * @param progressCallback A progress callback.
     */
    void SetProgressCallback( vtkSmartPointer<vtkCallbackCommand> progressCallback );

    /**
     * Loads the DICOM images within a directory.
     * If there are multiple DICOM sets, user interaction is needed.
     * @param pathToDicom Path to the DICOM directory.
     * @return DICOM image data.
     */
    vtkSmartPointer<vtkImageData> loadDicomImage( const std::string& pathToDicom );

    /**
     * Creates a mesh from out DICOM raw data.
     * @param imageData DICOM image data.
     * @param threshold Threshold for surface segmentation.
     * @return Resulting 3D mesh.
     */
    vtkSmartPointer<vtkPolyData> dicomToMesh( vtkSmartPointer<vtkImageData> imageData, const int& threshold );


    /**
     * Crop dicom images in terms of used slice ranges.
     * This starts a dialog with the user.
     * @param imageData
     */
    void cropDicom( vtkSmartPointer<vtkImageData> imageData );


private:

    vtkSmartPointer<vtkCallbackCommand> m_progressCallback;
};

#endif // _vtkDicomRoutines_H_
