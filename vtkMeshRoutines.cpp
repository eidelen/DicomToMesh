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

#include <vtkCenterOfMass.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkQuadricDecimation.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkSTLWriter.h>
#include <vtkCallbackCommand.h>
#include <vtkMath.h>

#include <string>
#include <iostream>
#include <fstream>

#include "vtkMeshRoutines.h"

using namespace std;

void VTKMeshRoutines::moveMeshToCOSCenter( vtkSmartPointer<vtkPolyData> mesh )
{
    vtkSmartPointer<vtkCenterOfMass> computeCenter = vtkSmartPointer<vtkCenterOfMass>::New();
    computeCenter->SetInputData( mesh );
    computeCenter->SetUseScalarsAsWeights(false);
    computeCenter->Update();

    double objectCenter[3];
    computeCenter->GetCenter(objectCenter);

    cout << "Move origin to center of mass: [" << objectCenter[0] << "," << objectCenter[1] << "," << objectCenter[2] << "]" << endl;

    vtkSmartPointer<vtkTransform> translation = vtkTransform::New();
    translation->Translate(-objectCenter[0], -objectCenter[1], -objectCenter[2]);

    vtkTransformPolyDataFilter* transformFilter = vtkTransformPolyDataFilter::New();
    transformFilter->SetInputData( mesh );
    transformFilter->SetTransform( translation );
    transformFilter->Update();

    mesh->DeepCopy( transformFilter->GetOutput() );

    // Free memory
    transformFilter->Delete();
    cout << endl << "Done" << endl << endl;
}

void VTKMeshRoutines::meshReduction( vtkSmartPointer<vtkPolyData> mesh, const float& reduction, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    string progressData = "Reduce mesh";
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    unsigned int numberOfCellsBefore = mesh->GetNumberOfCells();
    cout << "Mesh reduction by " << reduction << endl;

    // Note1: vtkQuadricDecimation seems to be better than vtkDecimatePro
    // Note2: vtkQuadricDecimation might have problem with face normals
    vtkQuadricDecimation* decimator = vtkQuadricDecimation::New();
    decimator->SetInputData( mesh );
    decimator->SetTargetReduction( reduction );
    decimator->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    decimator->Update();

    mesh->DeepCopy( decimator->GetOutput() );

    // Free memory
    decimator->Delete();

    unsigned int numberOfCellsAfter = mesh->GetNumberOfCells();
    cout << endl << "Mesh reduced from " << numberOfCellsBefore << " to " <<  numberOfCellsAfter << " faces" << endl;
    cout << "Done" << endl << endl;
}

void VTKMeshRoutines::removeSmallObjects( vtkSmartPointer<vtkPolyData> mesh, const float& ratio )
{
    cout << "Remove small connected objects: Size ratio = " << ratio << endl;

    vtkPolyDataConnectivityFilter* connectivityFilter = vtkPolyDataConnectivityFilter::New();
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

    // Free memory
    connectivityFilter->Delete();

    cout << "Done" << endl << endl;
}

//Todo: Understand FeatureAngle and RelaxationFactor. Then add it as argument.
void VTKMeshRoutines::smoothMesh( vtkSmartPointer<vtkPolyData> mesh, unsigned int nbrOfSmoothingIterations, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    string progressData = "Smooth mesh";
    progressCallback->SetClientData( (void*) (progressData.c_str()) );
    cout << "Mesh smoothing with " << nbrOfSmoothingIterations << " iterations." << endl;

    vtkSmoothPolyDataFilter* smoother = vtkSmoothPolyDataFilter::New();
    smoother->SetInputData( mesh );
    smoother->SetNumberOfIterations( nbrOfSmoothingIterations );
    smoother->SetFeatureAngle(45);
    smoother->SetRelaxationFactor(0.05);
    smoother->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    smoother->Update();

    mesh->DeepCopy( smoother->GetOutput() );

    // Free memory
    smoother->Delete();

    cout << endl << "Done" << endl << endl;
}

void VTKMeshRoutines::exportAsStlFile( const vtkSmartPointer<vtkPolyData>& mesh, const string& path )
{
    cout << "Mesh export as stl file: " << path << endl;
    vtkSmartPointer<vtkSTLWriter> writer = vtkSTLWriter::New();
    writer->SetFileName( path.c_str() );
    writer->SetInputData( mesh );
    writer->SetFileTypeToASCII();
    writer->Write();
    cout << "Done" << endl << endl;
}

void VTKMeshRoutines::computeVertexNormalsTrivial(const vtkSmartPointer<vtkPolyData>& mesh , std::vector<vtkVector3d> &normals)
{
    vtkSmartPointer<vtkPoints> vertices = mesh->GetPoints();
    vtkSmartPointer<vtkDataArray> verticesArray = vertices->GetData();

    long long numberOfVertices = vertices->GetNumberOfPoints();
    long long numberOfFaces = mesh->GetNumberOfCells();

    for( long long i = 0; i < numberOfVertices; i++ )
        normals.push_back( vtkVector3d(1,0,0) );

    // Go through all faces, compute face normal fn and set fn to all participating vertices.
    // The last computed fn of a vertex overwrites those before... that's why it is 'trivial' :)
    for( long long i = 0; i < numberOfFaces; i++ )
    {
        vtkSmartPointer<vtkIdList> face = vtkSmartPointer<vtkIdList>::New();
        mesh->GetCellPoints(i,face);
        long long v0Idx = face->GetId(0); long long v1Idx = face->GetId(1); long long v2Idx = face->GetId(2);

        vtkVector3d v0( verticesArray->GetComponent(v0Idx, 0), verticesArray->GetComponent(v0Idx, 1), verticesArray->GetComponent(v0Idx, 2) );
        vtkVector3d v1( verticesArray->GetComponent(v1Idx, 0), verticesArray->GetComponent(v1Idx, 1), verticesArray->GetComponent(v1Idx, 2) );
        vtkVector3d v2( verticesArray->GetComponent(v2Idx, 0), verticesArray->GetComponent(v2Idx, 1), verticesArray->GetComponent(v2Idx, 2) );

        // compute normal
        vtkVector3d v0v1; vtkMath::Subtract( v0.GetData(), v1.GetData(), v0v1.GetData() );
        vtkVector3d v0v2; vtkMath::Subtract( v0.GetData(), v2.GetData(), v0v2.GetData() );

        vtkVector3d fn = v0v1.Cross( v0v2 );
        fn.Normalize();

        normals.at(v0Idx) = fn;
        normals.at(v1Idx) = fn;
        normals.at(v2Idx) = fn;
    }
}

// Assembling the whole obj content first in memory and write at once to the file.
// (Observed file handles closing suddenly during obj export)
// Implementation partly from Mathias Griessen -> www.diffuse.ch
void VTKMeshRoutines::exportAsObjFile( const vtkSmartPointer<vtkPolyData>& mesh, const std::string& path )
{
    cout << "Mesh export as obj file: " << path << endl;

    string objContent("");
    char buffer[256];

    objContent.append( "#dicom2mesh obj exporter \n");

    vtkSmartPointer<vtkPoints> vertices = mesh->GetPoints();
    vtkSmartPointer<vtkDataArray> verticesArray = vertices->GetData();

    long long numberOfVertices = vertices->GetNumberOfPoints();
    long long  numberOfFaces = mesh->GetNumberOfCells();

    objContent.append("g default \n");

    // wrote vertices
    for( long long i = 0; i < numberOfVertices; i++ )
    {
        sprintf( buffer, "v %f %f %f \n",
                 verticesArray->GetComponent(i, 0), verticesArray->GetComponent(i, 1),
                 verticesArray->GetComponent(i, 2) );
        objContent.append(buffer);
    }

    // compute normals and write
    vector<vtkVector3d> normals;
    VTKMeshRoutines::computeVertexNormalsTrivial( mesh, normals );
    for(  long long i = 0; i < numberOfVertices; i++ )
    {
        vtkVector3d n = normals.at(i);
        sprintf( buffer, "vn %f %f %f \n", n.GetX(), n.GetY(), n.GetZ() );
        objContent.append(buffer);
    }

    objContent.append("\n");

    //faces
    objContent.append("g polyDefault \n");
    objContent.append("s off \n");

    for(unsigned int i = 0; i < numberOfFaces; i++)
    {
        vtkSmartPointer<vtkIdList> face = vtkSmartPointer<vtkIdList>::New();
        mesh->GetCellPoints(i,face);
        long long v0Idx = face->GetId(0); long long v1Idx = face->GetId(1); long long v2Idx = face->GetId(2);

        sprintf(buffer,"f %lld/%lld/%lld %lld/%lld/%lld %lld/%lld/%lld \n",
                                                          v0Idx+1, v0Idx+1, v0Idx+1,
                                                          v1Idx+1, v1Idx+1, v1Idx+1,
                                                          v2Idx+1, v2Idx+1, v2Idx+1);

        objContent.append(buffer);
    }

    objContent.append("\n\n#end of obj file\n");

    // write the whole buffer to the file
    ofstream objFile;
    objFile.open(path, ios::out);
    objFile.write( objContent.c_str(), objContent.length() );
    objFile.flush();

    cout << "Done" << endl << endl;
}
