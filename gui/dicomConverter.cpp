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

DicomConverter::DicomConverter(vtkSmartPointer<vtkCallbackCommand> cb)
{
    m_progressCB = cb;

    m_vdr = std::shared_ptr<VTKDicomRoutines>( new VTKDicomRoutines() );
    m_vdr->SetProgressCallback( m_progressCB );

    m_vmr = std::shared_ptr<VTKMeshRoutines>( new VTKMeshRoutines() );
    m_vmr->SetProgressCallback( m_progressCB );
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

void DicomConverter::centerMesh(bool doCentering )
{
    if( doCentering )
    {
        m_vmr->moveMeshToCOSCenter( m_mesh );
    }

    emit centerMesh_Done(true);
}
