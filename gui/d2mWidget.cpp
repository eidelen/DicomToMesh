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

#include "vtkMeshRoutines.h"
#include "vtkDicomRoutines.h"
#include "vtkMeshVisualizer.h"


#include <QFileDialog>
#include <vtkAlgorithm.h>

D2MWidget::D2MWidget(QWidget *parent) : QWidget(parent), ui(new Ui::D2MWidget)
{
    ui->setupUi(this);

    connect(ui->openBtn, SIGNAL(clicked()), this, SLOT(openDicomBtn()));
    connect(ui->saveBtn, SIGNAL(clicked()), this, SLOT(saveBtn()));
    connect(ui->runBtn, SIGNAL(clicked()), this, SLOT(runBtn()));
}

D2MWidget::~D2MWidget()
{
    delete ui;
}

void D2MWidget::openDicomBtn()
{
    QString dicomPath = QFileDialog::getExistingDirectory(this, "Choose DICOM directory", ".", QFileDialog::ShowDirsOnly);
    if(!dicomPath.isEmpty())
    {
        QString newInLablePath = "Input: ";
        newInLablePath.append(dicomPath);
        ui->inLable->setText(newInLablePath);

        m_dicom_path = dicomPath.toStdString();
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

        m_mesh_path = savePath.toStdString();
    }
}

void D2MWidget::runBtn()
{
    vtkSmartPointer<vtkCallbackCommand> progressCallbackVTK = vtkSmartPointer<vtkCallbackCommand>::New();
    progressCallbackVTK->SetCallback(progressCallback);

    //******** Read DICOM *********//
    bool loadSuccessful;
    vtkSmartPointer<vtkPolyData> mesh = loadInputData( m_dicom_path, 400, progressCallbackVTK, loadSuccessful );
    if( !loadSuccessful )
        return;
//    //******************************//
//
//    if( mesh->GetNumberOfCells() == 0 )
//    {
//        cerr << "No mesh could be created. Wrong DICOM or wrong iso value" << endl;
//        return -1;
//    }
//
//    //***** Mesh post-processing *****//
//    std::shared_ptr<VTKMeshRoutines> vmr = std::shared_ptr<VTKMeshRoutines>( new VTKMeshRoutines() );
//    vmr->SetProgressCallback( progressCallback );
//
//    if( settings.setOriginToCenterOfMass )
//        vmr->moveMeshToCOSCenter( mesh );
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

}

void D2MWidget::progressCallback(vtkObject* caller, long unsigned int /*eventId*/, void* clientData, void* /*callData*/)
{
    // display progress in terminal
    vtkAlgorithm* filter = static_cast<vtkAlgorithm*>(caller);
    char* task = static_cast<char*>(clientData);
    cout << "\33[2K\r"; // erase line
    cout << task << ": ";
    if( filter->GetProgress() > 0.999 )
        cout << "done";
    else
        std::cout << std::fixed << std::setprecision( 1 )  << filter->GetProgress() * 100 << "%";
    std::cout << std::flush;
}

vtkSmartPointer<vtkPolyData> D2MWidget::loadInputData( const std::string& path, const int& threshold, vtkSmartPointer<vtkCallbackCommand> progressCallback, bool& successful )
{
    vtkSmartPointer<vtkPolyData> mesh;

    std::shared_ptr<VTKDicomRoutines> vdr = std::shared_ptr<VTKDicomRoutines>( new VTKDicomRoutines() );
    vdr->SetProgressCallback( progressCallback );

    vtkSmartPointer<vtkImageData> imgData = vdr->loadDicomImage( path );
    if( imgData == NULL )
    {
        std::cerr << "No image data could be created. Maybe wrong directory?" << endl;
        successful = false;
    }
    else
    {
        /*
        if( settings.enableCrop )
            vdr->cropDicom( imgData );
         */

        mesh = vdr->dicomToMesh( imgData, threshold );
        successful = true;
    }

    return mesh;
}

