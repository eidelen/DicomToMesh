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
#include <vtkExtractVOI.h>
#include <iostream>
#include "vtkDicomRoutines.h"


using namespace std;

#ifdef USEVTKDICOM // advanced vtk dicom library. to be enabled in cmake.

#include "vtkDICOMDirectory.h"
#include "vtkDICOMItem.h"
#include "vtkStringArray.h"
#include "vtkDICOMReader.h"

vtkImageData* VTKDicomRoutines::loadDicomImage( const std::string& pathToDicom, vtkSmartPointer<vtkCallbackCommand> progressCallback )
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
        int nbrOfSlices_s = files_s->GetNumberOfValues();

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
        scanf("%d", &s_nbr);
        if( s_nbr < 0 || s_nbr >= nbrOfSeries )
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

    string progressData("Read DICOM");
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    vtkDICOMReader* reader = vtkDICOMReader::New();
    reader->SetFileNames( dicomDirectory->GetFileNamesForSeries( s_nbr ) );
    reader->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    reader->Update();

    vtkImageData* rawVolumeData = vtkImageData::New();
    rawVolumeData->DeepCopy(reader->GetOutput());

    reader->Delete(); // free memory
    cout << endl << endl;

    return rawVolumeData;
}

#else

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

#endif // USEVTKDICOM

vtkPolyData* VTKDicomRoutines::dicomToMesh( vtkSmartPointer<vtkImageData> imageData, const int& threshold, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    cout << "Create surface mesh with iso value = " << threshold << endl;
    string progressData = "Create mesh";
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    vtkMarchingCubes* surfaceExtractor = vtkMarchingCubes::New();
    surfaceExtractor->ComputeNormalsOn();
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

void VTKDicomRoutines::cropDicom( vtkSmartPointer<vtkImageData> imageData )
{
    int* inputImgDimension = imageData->GetDimensions();
    int xd = inputImgDimension[0]; int yd = inputImgDimension[1]; int zd = inputImgDimension[2];

    // ask user for slice range
    int startSlice, endSlice;
    cout << "Input image sclice range from   0 - " << zd << endl;
    cout << "Start slice = ";
    scanf("%d", &startSlice);
    cout << endl << "End slice = ";
    scanf("%d", &endSlice);
    cout << endl;

    // check passed slice values
    if( startSlice < 0 || startSlice > endSlice || endSlice >= zd )
    {
        cout << "Invalid slice settings - skip cropping." << endl;
    }
    else
    {
        cout << "Crop from slice " << startSlice << " to " << endSlice << endl;

        vtkSmartPointer<vtkExtractVOI> cropper = vtkSmartPointer<vtkExtractVOI>::New();
        cropper->SetInputData( imageData );
        cropper->SetVOI(0,xd-1, 0,yd-1, 200,300 );
        cropper->Update();
        imageData->DeepCopy( cropper->GetOutput() );
    }
}
