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


#include "d2mWidget.h"
#include "ui_d2mWidget.h"
#include "vtkMeshVisualizer.h"

#include <QFileDialog>
#include <vtkAlgorithm.h>


D2MWidget::D2MWidget(QWidget *parent) : QWidget(parent), ui(new Ui::D2MWidget)
{
    ui->setupUi(this);

    connect(ui->openBtn, SIGNAL(clicked()), this, SLOT(openDicomBtn()));
    connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(saveBtn()));
    connect(ui->runBtn, SIGNAL(clicked()), this, SLOT(runBtn()));
    connect(this, &D2MWidget::doUpdateProgress, this, &D2MWidget::updateProgress);

    // conversion thread
    m_converter = new DicomConverter(this);
    m_converter->moveToThread(&m_conversionThread);
    connect(&m_conversionThread, &QThread::finished, m_converter, &QObject::deleteLater);

    connect(this, &D2MWidget::doLoad, m_converter, &DicomConverter::loadDicomImage);
    connect(m_converter, &DicomConverter::loadDicomImage_Done, this, &D2MWidget::load_done);

    connect(this, &D2MWidget::doCenter, m_converter, &DicomConverter::centerMesh);
    connect(m_converter, &DicomConverter::centerMesh_Done, this, &D2MWidget::center_done);

    connect(this, &D2MWidget::doReduction, m_converter, &DicomConverter::reduction);
    connect(m_converter, &DicomConverter::reduction_Done, this, &D2MWidget::reduction_done);

    connect(this, &D2MWidget::doFilter, m_converter, &DicomConverter::filtering);
    connect(m_converter, &DicomConverter::filtering_Done, this, &D2MWidget::filter_done);

    connect(this, &D2MWidget::doSmoothing, m_converter, &DicomConverter::smoothing);
    connect(m_converter, &DicomConverter::smoothing_Done, this, &D2MWidget::smoothing_done);

    connect(this, &D2MWidget::doExport, m_converter, &DicomConverter::exportMesh);
    connect(m_converter, &DicomConverter::export_Done, this, &D2MWidget::export_done);

    m_conversionThread.start();

    m_mesh_path = "";
    m_dicom_path = "";
}

D2MWidget::~D2MWidget()
{
    delete ui;

    m_conversionThread.quit();
    m_conversionThread.wait();
}

void D2MWidget::openDicomBtn()
{
    QString dicomPath = QFileDialog::getExistingDirectory(this, "Choose DICOM directory", ".", QFileDialog::ShowDirsOnly);
    if(!dicomPath.isEmpty())
    {
        QString newInLablePath = "Input: ";
        newInLablePath.append(dicomPath);
        ui->inLable->setText(newInLablePath);

        m_dicom_path = dicomPath;
    }
}

void D2MWidget::saveBtn()
{
    QString savePath = QFileDialog::getSaveFileName(this, "Save mesh as", ".");
    if(!savePath.isEmpty())
    {
        QString newOutLable = "Output: ";
        newOutLable.append(savePath);
        ui->outLable->setText(newOutLable);

        m_mesh_path = savePath;
    }
}

void D2MWidget::runBtn()
{
    handleStartConversion();

    // Read Dicom images
    ui->infoLable->setText("Read DICOM images");
    emit doLoad( m_dicom_path, ui->thresholdSpinBox->value());

    // after dicom images loaded, slot load_done is called
}

void D2MWidget::load_done(bool ok)
{
    if( ok )
    {
        if( ui->centerCB->isChecked() )
        {
            ui->infoLable->setText("Center mesh");
            emit doCenter();

            // when done, center_done is called
        }
        else
        {
            // jump directly to center_done
            center_done(true);
        }
    }
    else
    {
        ui->infoLable->setText("Read DICOM failed");
        handleEndConversion();
    }
}

void D2MWidget::center_done(bool ok)
{
    if( ok )
    {
        if( ui->reduceCB->isChecked() )
        {
            ui->infoLable->setText("Mesh reduction");
            emit doReduction(ui->reduceSP->value());

            // after reducing, reduction_done is called
        }
        else
        {
            // jump directly to center_done
            reduction_done(true);
        }
    }
    else
    {
        ui->infoLable->setText("Center mesh failed");
        handleEndConversion();
    }
}

void D2MWidget::reduction_done(bool ok)
{
    if( ok )
    {
        if(ui->filterCB->isChecked())
        {
            ui->infoLable->setText("Filter small objects");
            emit doFilter(ui->filterSP->value());

            // after filtering, filter_done is called
        }
        else
        {
            // jump directly to center_done
            filter_done(true);
        }
    }
    else
    {
        ui->infoLable->setText("Reduction failed");
        handleEndConversion();
    }
}

void D2MWidget::filter_done(bool ok)
{
    if( ok )
    {
        if( ui->smoothCB->isChecked() )
        {
            ui->infoLable->setText("Smoothing mesh");
            emit doSmoothing();

            // after smooting, smoothing_done is called
        }
        else
        {
            // jump directly to center_done
            smoothing_done(true);
        }
    }
    else
    {
        ui->infoLable->setText("Filtering failed");
        handleEndConversion();
    }
}

void D2MWidget::smoothing_done(bool ok)
{
    if( ok )
    {
        if( m_mesh_path != "" )
        {
            ui->infoLable->setText("Export mesh file");
            emit doExport(m_mesh_path);

            // after exporting, export_done cis alled
        }
        else
        {
            export_done(true);
        }
    }
    else
    {
        ui->infoLable->setText("Smoothing failed");
        handleEndConversion();
    }
}

void D2MWidget::export_done(bool ok)
{
    if( ok )
    {
        ui->infoLable->setText("Visualize");
        VTKMeshVisualizer::displayMesh( m_converter->getMesh() );
        handleEndConversion();
    }
    else
    {
        ui->infoLable->setText("Export failed");
        handleEndConversion();
    }
}

void D2MWidget::visualize_done(bool ok)
{
    ui->infoLable->setText("Done");
    handleEndConversion();
}

void D2MWidget::converterProgress(float progress)
{
    emit doUpdateProgress(progress);
}

void D2MWidget::updateProgress(float progress)
{
    ui->progressBar->setValue(static_cast<int>(progress));
}

void D2MWidget::handleStartConversion()
{
    ui->runBtn->setEnabled(false);
}

void D2MWidget::handleEndConversion()
{
    ui->runBtn->setEnabled(true);
}







