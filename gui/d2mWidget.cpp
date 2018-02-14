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

    m_conversionThread.start();
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
    // Read Dicom images
    ui->infoLable->setText("Read DICOM images");
    ui->progressBar->setValue(0);
    emit doLoad( m_dicom_path, ui->thresholdSpinBox->value());

    // after dicom images loaded, slot load_done is called
}

void D2MWidget::load_done(bool ok)
{
    if( ok )
    {
        ui->infoLable->setText("Center mesh");
        emit doCenter(ui->centerCB->isChecked());

        // when done, center_done is called
    }
    else
    {
        ui->infoLable->setText("Read DICOM failed");
    }
}

void D2MWidget::center_done(bool ok)
{
    if( ok )
    {
        ui->infoLable->setText("Mesh reduction");
        emit doReduction(ui->reduceCB->isChecked(), ui->reduceSP->value());

        // after reducing, reduction_done is called
    }
    else
    {
        ui->infoLable->setText("Center mesh failed");
    }
}

void D2MWidget::reduction_done(bool ok)
{
    if( ok )
    {
        ui->infoLable->setText("Reduction done");
    }
    else
    {
        ui->infoLable->setText("Reduction failed");
    }
}

/*
    if( !loadDicomImage() )
    {
        return;
    }*/

//    //******************************//
//
//
//
//    if( settings.enableMeshReduction )
//    {
//        // check reduction rate
//        if( settings.reductionRate < 0.0 || settings.reductionRate > 1.0 )
//            cout << "Reduction skipped due to invalid reductionRate " << settings.reductionRate << " where a value of 0.0 - 1.0 is expected." << endl;
//        else
//            vmr->meshReduction( mesh, settings.reductionRate );
//    }
//
//    if( settings.enablePolygonLimitation )
//    {
//        if( mesh->GetNumberOfCells() > vtkIdType(settings.polygonLimit) )
//        {
//            double reductionRate = 1.0 - (  (double(settings.polygonLimit)) / (double(mesh->GetNumberOfCells()))) ;
//            vmr->meshReduction( mesh, reductionRate );
//        }
//        else
//        {
//            cout << "Reducing polygons not necessary." << endl << endl;
//        }
//    }
//
//    if( settings.extracOnlyBigObjects )
//    {
//        if( settings.nbrVerticesRatio < 0.0 || settings.nbrVerticesRatio > 1.0 )
//            cout << "Smoothing skipped due to invalid reductionRate " << settings.nbrVerticesRatio << " where a value of 0.0 - 1.0 is expected." << endl;
//        else
//            vmr->removeSmallObjects( mesh, settings.nbrVerticesRatio );
//    }
//
//    if( settings.enableSmoothing )
//    {
//        vmr->smoothMesh( mesh, 20 );
//    }
//
//    //********************************//
//
//    // check if obj or stl
//    if( settings.enabledExportMeshFile )
//    {
//        string::size_type idx = settings.outputFilePath.rfind('.');
//        if( idx != string::npos )
//        {
//            string extension = settings.outputFilePath.substr(idx+1);
//
//            if( extension == "obj" )
//                vmr->exportAsObjFile( mesh, settings.outputFilePath );
//            else if( extension == "stl" )
//                vmr->exportAsStlFile( mesh, settings.outputFilePath );
//            else if( extension == "ply" )
//                vmr->exportAsPlyFile( mesh, settings.outputFilePath );
//            else
//                cerr << "Unknown file type" << endl;
//        }
//        else
//        {
//            cerr << "No Filename." << endl;
//        }
//    }
//
//    if( settings.showIn3DView )
//        VTKMeshVisualizer::displayMesh( mesh );


void D2MWidget::converterProgress(float progress)
{
    emit doUpdateProgress(progress);
}

void D2MWidget::updateProgress(float progress)
{
    ui->progressBar->setValue(static_cast<int>(progress));
}


