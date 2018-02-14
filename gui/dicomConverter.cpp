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

#include "dicomConverter.h"

#include <vtkAlgorithm.h>
#include <string>

DicomConverter::DicomConverter(DicomConverter_Listener* host)
{
    m_host = host;

    m_progressCB = vtkSmartPointer<vtkCallbackCommand>::New();
    m_progressCB->SetCallback(progressCallback);
    m_progressCB->SetClientData(static_cast<void*>(m_host));

    m_vdr = std::shared_ptr<VTKDicomRoutines>( new VTKDicomRoutines() );
    m_vdr->SetProgressCallback( m_progressCB );

    m_vmr = std::shared_ptr<VTKMeshRoutines>( new VTKMeshRoutines() );
    m_vmr->SetProgressCallback( m_progressCB );
}

void DicomConverter::progressCallback(vtkObject* caller, long unsigned int /*eventId*/, void* clientData, void* /*callData*/)
{
    // display progress in terminal
    vtkAlgorithm* filter = static_cast<vtkAlgorithm*>(caller);

    DicomConverter_Listener* host = static_cast<DicomConverter_Listener*>(clientData);
    host->converterProgress( filter->GetProgress() * 100);
}


void DicomConverter::loadDicomImage( const QString& pathToDicom, int threshold )
{
    bool ret;

    std::cout << "loadDicomImage" << std::endl;

    vtkSmartPointer<vtkImageData> imgData = m_vdr->loadDicomImage( pathToDicom.toStdString() );
    if( imgData == NULL )
    {
        std::cerr << "No DICOM data could be found in the directory" << endl;
        ret = false;
    }
    else
    {
        m_mesh = m_vdr->dicomToMesh( imgData, threshold);

        if( m_mesh->GetNumberOfCells() == 0 )
        {
            std::cerr << "No mesh could be created. Wrong DICOM or wrong iso value" << endl;
            ret = false;
        }
        else
        {
            ret = true;
        }
    }

    emit loadDicomImage_Done(ret);
}

void DicomConverter::centerMesh()
{
    m_vmr->moveMeshToCOSCenter( m_mesh );
    emit centerMesh_Done(true);
}

void DicomConverter::reduction(float reductionRate)
{
    bool ret = true;

    if( reductionRate > 0.0 && reductionRate < 1.0 )
    {
        m_vmr->meshReduction(m_mesh, reductionRate);
        ret = true;
    }
    else
    {
        std::cerr << "Invalid reduction rate: " << reductionRate << endl;
        ret = false;
    }

    emit reduction_Done(ret);
}

void DicomConverter::filtering(float filterRate )
{
    bool ret = true;

    if( filterRate > 0.0 && filterRate < 1.0 )
    {
        m_vmr->removeSmallObjects(m_mesh, filterRate);
        ret = true;
    }
    else
    {
        std::cerr << "Invalid filter rate: " << filterRate << endl;
        ret = false;
    }

    emit filtering_Done(ret);
}

void DicomConverter::smoothing()
{
    bool ret = true;

    m_vmr->smoothMesh(m_mesh, 20);

    emit smoothing_Done(ret);
}

void DicomConverter::exportMesh(const QString &meshPath)
{
    bool ret = true;

    std::string path = meshPath.toStdString();
    std::string::size_type idx = path.rfind('.');
    if( idx != std::string::npos )
    {
        std::string extension = path.substr(idx+1);

        if( extension == "obj" )
        {
            m_vmr->exportAsObjFile(m_mesh, path);
        }
        else if( extension == "stl" )
        {
            m_vmr->exportAsStlFile( m_mesh, path );
        }
        else if( extension == "ply" )
        {
            m_vmr->exportAsPlyFile( m_mesh, path );
        }
        else
        {
            std::cerr << "Unknown file type" << endl;
            ret = false;
        }
    }
    else
    {
        std::cerr << "No Filename." << endl;
        ret = false;
    }

    emit export_Done(ret);
}
