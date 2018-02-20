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

#include "vtkDicomRoutines.h"

#include <vtkDICOMImageReader.h>
#include <vtkMarchingCubes.h>
#include <vtkExtractVOI.h>
#include <iostream>

using namespace std;

VTKDicomRoutines::VTKDicomRoutines()
{
    m_progressCallback = vtkSmartPointer<vtkCallbackCommand>(NULL);
}

VTKDicomRoutines::~VTKDicomRoutines()
{
}

void VTKDicomRoutines::SetProgressCallback( vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    m_progressCallback = progressCallback;
}


#ifdef USEVTKDICOM // advanced vtk dicom library. to be enabled in cmake.

#include "vtkDICOMDirectory.h"
#include "vtkDICOMItem.h"
#include "vtkStringArray.h"
#include "vtkDICOMReader.h"

vtkSmartPointer<vtkImageData> VTKDicomRoutines::loadDicomImage( const std::string& pathToDicom )
{
    // analyze dicom directory. there might be mulitple data
    vtkSmartPointer<vtkDICOMDirectory> dicomDirectory = vtkSmartPointer<vtkDICOMDirectory>::New();
    dicomDirectory->SetDirectoryName(pathToDicom.c_str());
    dicomDirectory->SetScanDepth(1);
    dicomDirectory->Update();

    cout << "Read DICOM images located under " << pathToDicom << endl;
    cout << "Nbr of patients = "<< dicomDirectory->GetNumberOfPatients() << ", ";
    cout << "Nbr of studies = " << dicomDirectory->GetNumberOfStudies() << ", ";

    const int& nbrOfSeries = dicomDirectory->GetNumberOfSeries();
    cout << "Nbr of series = "<< nbrOfSeries << endl;
    for( int s = 0; s < nbrOfSeries; s++ )
    {
        const vtkDICOMItem& dicomSeries_s = dicomDirectory->GetSeriesRecord( s );
        vtkStringArray* files_s = dicomDirectory->GetFileNamesForSeries( s );
        vtkIdType nbrOfSlices_s = files_s->GetNumberOfValues();

        cout << "(" << s << ")  :  " << nbrOfSlices_s << " files, name = " << dicomSeries_s.Get(DC::SeriesDescription).AsString() << endl;
    }


    // choose a particular dicom serie
    int s_nbr;
    if( nbrOfSeries == 1 ) // only one
    {
        s_nbr = 0;
    }
    else if( nbrOfSeries > 1 ) // multiple dicom series
    {
        cout << "Which DICOM series you wish to load? ";
        int scanRes = std::scanf("%d", &s_nbr);
        if( scanRes == 1 && (s_nbr < 0 || s_nbr >= nbrOfSeries) )
        {
            cerr << "Wrong DICOM serie index" << endl;
            return NULL;
        }
    }
    else
    {
        cerr << "No DICOM data in directory" << endl;
        return NULL;
    }

    // load dicom serie
    const vtkDICOMItem& selected_serie = dicomDirectory->GetSeriesRecord( s_nbr );
    cout << endl << "Load serie " << s_nbr << ", " << selected_serie.Get(DC::SeriesDescription).AsString() << endl;

    vtkSmartPointer<vtkDICOMReader> reader = vtkSmartPointer<vtkDICOMReader>::New();
    reader->SetFileNames( dicomDirectory->GetFileNamesForSeries( s_nbr ) );
    if( m_progressCallback.Get() != NULL )
    {
        reader->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    reader->Update();

    vtkSmartPointer<vtkImageData> rawVolumeData = vtkSmartPointer<vtkImageData>::New();
    rawVolumeData->DeepCopy(reader->GetOutput());

    cout << endl << endl;
    return rawVolumeData;
}

#else

vtkSmartPointer<vtkImageData> VTKDicomRoutines::loadDicomImage( const std::string& pathToDicom )
{
    cout << "Read DICOM images located under " << pathToDicom << endl;

    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName( pathToDicom.c_str() );
    if( m_progressCallback.Get() != NULL )
    {
        reader->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    reader->Update();

    vtkSmartPointer<vtkImageData> rawVolumeData = vtkSmartPointer<vtkImageData>::New();
    rawVolumeData->DeepCopy(reader->GetOutput());

    // check if load was successful
    int* dims = rawVolumeData->GetDimensions();
    if( dims[0] < 1 || dims[1] < 1 || dims[2] < 1 )
    {
        cerr << "No DICOM data in directory" << endl;
        return NULL;
    }

    cout << endl << endl;

    return rawVolumeData;
}

#endif // USEVTKDICOM

vtkSmartPointer<vtkPolyData> VTKDicomRoutines::dicomToMesh( vtkSmartPointer<vtkImageData> imageData, const int& threshold )
{
    cout << "Create surface mesh with iso value = " << threshold << endl;

    vtkSmartPointer<vtkMarchingCubes> surfaceExtractor = vtkSmartPointer<vtkMarchingCubes>::New();
    surfaceExtractor->ComputeNormalsOn();
    surfaceExtractor->SetValue( 0,threshold ) ;
    surfaceExtractor->SetInputData( imageData );
    if( m_progressCallback.Get() != NULL )
    {
        surfaceExtractor->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    surfaceExtractor->Update();

    vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
    mesh->DeepCopy( surfaceExtractor->GetOutput() );

    cout << endl << endl;
    return mesh;
}

void VTKDicomRoutines::cropDicom( vtkSmartPointer<vtkImageData> imageData )
{
    int* inputImgDimension = imageData->GetDimensions();
    int xd = inputImgDimension[0]; int yd = inputImgDimension[1]; int zd = inputImgDimension[2];

    // ask user for slice range
    int startSlice, endSlice;
    cout << "Input image sclice range from   0 - " << zd << endl;
    cout << "Start slice = ";
    int startSliceScanRes = std::scanf("%d", &startSlice);
    cout << endl << "End slice = ";
    int endSliceScanRes = std::scanf("%d", &endSlice);
    cout << endl;

    // check passed slice values
    if( startSliceScanRes != 1 || endSliceScanRes != 1 || startSlice < 0 || startSlice > endSlice || endSlice >= zd )
    {
        cout << "Invalid slice settings - skip cropping." << endl;
    }
    else
    {
        cout << "Crop from slice " << startSlice << " to " << endSlice << endl;

        vtkSmartPointer<vtkExtractVOI> cropper = vtkSmartPointer<vtkExtractVOI>::New();
        cropper->SetInputData( imageData );
        cropper->SetVOI(0,xd-1, 0,yd-1, startSlice, endSlice );
        if( m_progressCallback.Get() != NULL )
        {
            cropper->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
        }
        cropper->Update();
        imageData->DeepCopy( cropper->GetOutput() );

        cout << endl << endl;
    }
}
