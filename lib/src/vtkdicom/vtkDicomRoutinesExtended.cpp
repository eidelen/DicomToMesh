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

#include "vtkDicomRoutinesExtended.h"

#include <iostream>
#include "vtkDICOMDirectory.h"
#include "vtkDICOMItem.h"
#include "vtkStringArray.h"
#include "vtkDICOMReader.h"

using namespace std;

VTKDicomRoutinesExtended::VTKDicomRoutinesExtended()
{
}

VTKDicomRoutinesExtended::~VTKDicomRoutinesExtended()
{
}

vtkSmartPointer<vtkImageData> VTKDicomRoutinesExtended::loadDicomImage( const std::string& pathToDicom )
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


