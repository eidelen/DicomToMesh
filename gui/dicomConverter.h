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


#ifndef QDICOM_CONVERTER_H
#define QDICOM_CONVERTER_H

#include "vtkDicomRoutines.h"
#include "vtkMeshRoutines.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCallbackCommand.h>

#include <QObject>

#include <memory>

class DicomConverter_Listener
{
public:
    virtual void converterProgress(float progress) = 0;
};

class DicomConverter : public QObject
{
Q_OBJECT

public:
    DicomConverter(DicomConverter_Listener* host);

    static void progressCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

    vtkSmartPointer<vtkPolyData> getMesh();

public slots:
    void loadDicomImage( const QString& pathToDicom, int threshold );
    void centerMesh();
    void reduction(float reductionRate);
    void filtering(float filterRate);
    void smoothing();
    void exportMesh(const QString& meshPath);

signals:
    void loadDicomImage_Done(bool ok);
    void centerMesh_Done(bool ok);
    void reduction_Done(bool ok);
    void filtering_Done(bool ok);
    void smoothing_Done(bool ok);
    void export_Done(bool ok);

private:
    DicomConverter_Listener* m_host;
    vtkSmartPointer<vtkCallbackCommand> m_progressCB;
    vtkSmartPointer<vtkPolyData> m_mesh;
    std::shared_ptr<VTKDicomRoutines> m_vdr;
    std::shared_ptr<VTKMeshRoutines> m_vmr;
};

#endif // QDICOM_CONVERTER_H
