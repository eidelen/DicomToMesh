/****************************************************************************
** Copyright (c) 2017 Adrian Schneider, https://github.com/eidelen
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


#ifndef WIDGET_H
#define WIDGET_H

#include "dicomConverter.h"

#include <vtkCallbackCommand.h>
#include <QWidget>
#include <QThread>


namespace Ui {
    class D2MWidget;
}
 
class D2MWidget : public QWidget, public DicomConverter_Listener
{
    Q_OBJECT
 
public:
    explicit D2MWidget(QWidget *parent = 0);
    ~D2MWidget();

    // from DicomConverter_Listener
    void converterProgress(double progress);

public slots:
    void openDicomBtn();
    void saveBtn();
    void runBtn();
    void updateProgress(double progress);

    void load_done(bool ok);
    void center_done(bool ok);
    void reduction_done(bool ok);
    void filter_done(bool ok);
    void smoothing_done(bool ok);
    void export_done(bool ok);

signals:
    void doLoad(const QString& pathToDicom, int threshold);
    void doCenter();
    void doReduction(double reductionRate);
    void doFilter(double filterRate);
    void doSmoothing();
    void doExport(const QString& path);

    void doUpdateProgress(double progress);

private:
    void handleStartConversion();
    void handleEndConversion();

private:
    Ui::D2MWidget *ui;

    QString m_dicom_path;
    QString m_mesh_path;
    vtkSmartPointer<vtkCallbackCommand> m_progressCB;
    QThread m_conversionThread;
    DicomConverter* m_converter;
};

 
#endif // WIDGET_H
