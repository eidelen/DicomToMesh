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

#include "meshRoutines.h"

#include <vtkCenterOfMass.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkQuadricDecimation.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>

#include <iostream>
#include <fstream>


using namespace std;

VTKMeshRoutines::VTKMeshRoutines()
{
    m_progressCallback = vtkSmartPointer<vtkCallbackCommand>(NULL);
}

VTKMeshRoutines::~VTKMeshRoutines()
{
}

void VTKMeshRoutines::SetProgressCallback( vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    m_progressCallback = progressCallback;
}

vtkVector3d VTKMeshRoutines::moveMeshToCOSCenter( vtkSmartPointer<vtkPolyData> mesh )
{
    vtkSmartPointer<vtkCenterOfMass> computeCenter = vtkSmartPointer<vtkCenterOfMass>::New();
    computeCenter->SetInputData( mesh );
    computeCenter->SetUseScalarsAsWeights(false);
    computeCenter->Update();

    double* objectCenter = new double[3];
    computeCenter->GetCenter(objectCenter);

    // safe to returning vector
    vtkVector3d retV(-objectCenter[0],-objectCenter[1],-objectCenter[2]);

    vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(-objectCenter[0], -objectCenter[1], -objectCenter[2]);

    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData( mesh );
    transformFilter->SetTransform( translation );
    transformFilter->Update();

    mesh->DeepCopy( transformFilter->GetOutput() );

    // Free memory
    delete[] objectCenter;

    cout << "Done" << endl << endl;

    return retV;
}

void VTKMeshRoutines::meshReduction( vtkSmartPointer<vtkPolyData> mesh, double reduction )
{
    long long numberOfCellsBefore = mesh->GetNumberOfCells();
    cout << "Mesh reduction by " << std::fixed << std::setprecision( 3 ) << reduction << endl;

    // Note1: vtkQuadricDecimation seems to be better than vtkDecimatePro
    // Note2: vtkQuadricDecimation might have problem with face normals
    vtkSmartPointer<vtkQuadricDecimation> decimator = vtkSmartPointer<vtkQuadricDecimation>::New();
    decimator->SetInputData( mesh );
    decimator->SetTargetReduction( reduction );
    if( m_progressCallback.Get() != NULL )
    {
        decimator->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    decimator->Update();

    mesh->DeepCopy( decimator->GetOutput() );

    long long numberOfCellsAfter = mesh->GetNumberOfCells();
    cout << endl << "Mesh reduced from " << numberOfCellsBefore << " to " <<  numberOfCellsAfter << " faces" << endl;
    cout << endl << endl;
}

void VTKMeshRoutines::removeSmallObjects( vtkSmartPointer<vtkPolyData> mesh, double ratio )
{
    cout << "Remove small connected objects: Size ratio = " << std::fixed << std::setprecision( 3 ) << ratio << endl;

    vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connectivityFilter->SetInputData( mesh );
    connectivityFilter->SetExtractionModeToAllRegions();
    connectivityFilter->Update();

    // remove objects consisting of less than ratio vertexes of the biggest object
    vtkIdTypeArray* regionSizes = connectivityFilter->GetRegionSizes();

    // find object with most vertices
    long maxSize = 0;
    for( int regions = 0; regions < connectivityFilter->GetNumberOfExtractedRegions(); regions++ )
        if( regionSizes->GetValue(regions) > maxSize )
            maxSize = regionSizes->GetValue(regions);


    // append regions of sizes over the threshold
    connectivityFilter->SetExtractionModeToSpecifiedRegions();
    for( int regions = 0; regions < connectivityFilter->GetNumberOfExtractedRegions(); regions++ )
        if( regionSizes->GetValue(regions) > maxSize * ratio )
            connectivityFilter->AddSpecifiedRegion(regions);

    connectivityFilter->Update();

    mesh->DeepCopy( connectivityFilter->GetOutput() );
    cout << "Done" << endl << endl << endl;
}

//Todo: Understand FeatureAngle and RelaxationFactor. Then add it as argument.
void VTKMeshRoutines::smoothMesh( vtkSmartPointer<vtkPolyData> mesh, unsigned int nbrOfSmoothingIterations )
{
    cout << "Mesh smoothing with " << nbrOfSmoothingIterations << " iterations." << endl;

    vtkSmartPointer<vtkSmoothPolyDataFilter> smoother = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smoother->SetInputData( mesh );
    smoother->SetNumberOfIterations( int(nbrOfSmoothingIterations) );
    smoother->SetFeatureAngle(45);
    smoother->SetRelaxationFactor(0.05);
    if( m_progressCallback.Get() != NULL )
    {
        smoother->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    smoother->Update();

    mesh->DeepCopy( smoother->GetOutput() );
    cout << endl << endl;
}
