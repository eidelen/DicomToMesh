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

#include <vtkDICOMImageReader.h>
#include <vtkMarchingCubes.h>
#include <iostream>
#include "vtkDicomRoutines.h"

using namespace std;

vtkImageData* VTKDicomRoutines::loadDicomImage( const std::string& pathToDicom, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    cout << "Read DICOM images located under " << pathToDicom << endl;
    string progressData("Read DICOM");
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    vtkDICOMImageReader* reader = vtkDICOMImageReader::New();
    reader->SetDirectoryName( pathToDicom.c_str() );
    reader->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    reader->Update();

    vtkImageData* rawVolumeData = vtkImageData::New();
    rawVolumeData->DeepCopy(reader->GetOutput());

    reader->Delete(); // free memory
    cout << endl << endl;

    return rawVolumeData;
}

vtkPolyData* VTKDicomRoutines::dicomToMesh( vtkSmartPointer<vtkImageData> imageData, const int& threshold, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    cout << "Create surface mesh with iso value = " << threshold << endl;
    string progressData = "Create mesh";
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    vtkMarchingCubes* surfaceExtractor = vtkMarchingCubes::New();
    surfaceExtractor->ComputeNormalsOn();
    surfaceExtractor->ComputeScalarsOn();
    surfaceExtractor->SetValue( 0,threshold ) ;
    surfaceExtractor->SetInputData( imageData );
    surfaceExtractor->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    surfaceExtractor->Update();

    vtkPolyData* mesh = vtkPolyData::New();
    mesh->DeepCopy( surfaceExtractor->GetOutput() );

    // free memory
    surfaceExtractor->Delete();
    cout << endl << endl;

    return mesh;
}
