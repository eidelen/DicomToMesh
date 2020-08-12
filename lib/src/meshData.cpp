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

#include "meshData.h"

#include <vtkSTLWriter.h>
#include <vtkPLYWriter.h>
#include <vtkMath.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkPLYReader.h>
#include <vtkTypedArray.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>

#include <iostream>
#include <fstream>


using namespace std;

VTKMeshData::VTKMeshData()
{
    m_progressCallback = vtkSmartPointer<vtkCallbackCommand>(NULL);
}

VTKMeshData::~VTKMeshData()
{
}

void VTKMeshData::SetProgressCallback( vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    m_progressCallback = progressCallback;
}

void VTKMeshData::computeVertexNormalsTrivial(const vtkSmartPointer<vtkPolyData>& mesh , std::vector<vtkVector3d> &normals)
{
    vtkSmartPointer<vtkPoints> vertices = mesh->GetPoints();
    vtkSmartPointer<vtkDataArray> verticesArray = vertices->GetData();

    vtkIdType numberOfVertices = vertices->GetNumberOfPoints();
    vtkIdType numberOfFaces = mesh->GetNumberOfCells();

    for( vtkIdType i = 0; i < numberOfVertices; i++ )
        normals.push_back( vtkVector3d(1,0,0) );

    // Go through all faces, compute face normal fn and set fn to all participating vertices.
    // The last computed fn of a vertex overwrites those before... that's why it is 'trivial' :)
    for( vtkIdType i = 0; i < numberOfFaces; i++ )
    {
        vtkSmartPointer<vtkIdList> face = vtkSmartPointer<vtkIdList>::New();
        mesh->GetCellPoints(i,face);
        vtkIdType v0Idx = face->GetId(0);
        vtkIdType v1Idx = face->GetId(1);
        vtkIdType v2Idx = face->GetId(2);

        vtkVector3d v0( verticesArray->GetComponent(v0Idx, 0), verticesArray->GetComponent(v0Idx, 1), verticesArray->GetComponent(v0Idx, 2) );
        vtkVector3d v1( verticesArray->GetComponent(v1Idx, 0), verticesArray->GetComponent(v1Idx, 1), verticesArray->GetComponent(v1Idx, 2) );
        vtkVector3d v2( verticesArray->GetComponent(v2Idx, 0), verticesArray->GetComponent(v2Idx, 1), verticesArray->GetComponent(v2Idx, 2) );

        // compute normal
        vtkVector3d v0v1; vtkMath::Subtract( v0.GetData(), v1.GetData(), v0v1.GetData() );
        vtkVector3d v0v2; vtkMath::Subtract( v0.GetData(), v2.GetData(), v0v2.GetData() );

        vtkVector3d fn = v0v1.Cross( v0v2 );
        fn.Normalize();

        normals.at( size_t(v0Idx) ) = fn;
        normals.at( size_t(v1Idx) ) = fn;
        normals.at( size_t(v2Idx) ) = fn;
    }
}

// Assembling the whole obj content first in memory and write at once to the file.
// (Observed file handles closing suddenly during obj export)
// Implementation partly from Mathias Griessen -> www.diffuse.ch
void VTKMeshData::exportAsObjFile( const vtkSmartPointer<vtkPolyData>& mesh, const std::string& path )
{
    cout << "Mesh export as obj file: " << path << endl;

    string objContent("");
    char* buffer = new char[256];

    objContent.append( "#dicom2mesh obj exporter \n");

    vtkSmartPointer<vtkPoints> vertices = mesh->GetPoints();
    vtkSmartPointer<vtkDataArray> verticesArray = vertices->GetData();

    vtkIdType numberOfVertices = vertices->GetNumberOfPoints();
    vtkIdType  numberOfFaces = mesh->GetNumberOfCells();

    objContent.append("g default \n");

    // wrote vertices
    for( vtkIdType i = 0; i < numberOfVertices; i++ )
    {
        sprintf( buffer, "v %f %f %f \n",
                 verticesArray->GetComponent(i, 0),
                 verticesArray->GetComponent(i, 1),
                 verticesArray->GetComponent(i, 2) );
        objContent.append(buffer);
    }

    // compute normals and write
    vector<vtkVector3d> normals;
    VTKMeshData::computeVertexNormalsTrivial( mesh, normals );
    for( vtkIdType i = 0; i < numberOfVertices; i++ )
    {
        vtkVector3d n = normals.at(size_t(i));
        sprintf( buffer, "vn %f %f %f \n", n.GetX(), n.GetY(), n.GetZ() );
        objContent.append(buffer);
    }

    objContent.append("\n");

    //faces
    objContent.append("g polyDefault \n");
    objContent.append("s off \n");

    for(long long i = 0; i < numberOfFaces; i++)
    {
        vtkSmartPointer<vtkIdList> face = vtkSmartPointer<vtkIdList>::New();
        mesh->GetCellPoints(i,face);
        long long v0Idx = face->GetId(0); long long v1Idx = face->GetId(1); long long v2Idx = face->GetId(2);

        sprintf(buffer,"f %lld//%lld %lld//%lld %lld//%lld \n",
                v0Idx+1, v0Idx+1,
                v1Idx+1, v1Idx+1,
                v2Idx+1, v2Idx+1);

        objContent.append(buffer);
    }

    objContent.append("\n\n#end of obj file\n");

    // write the whole buffer to the file
    ofstream objFile;
    objFile.open(path, ios::out);
    objFile.write( objContent.c_str(), long(objContent.length()) );
    objFile.flush();

    delete[] buffer;

    cout << "Done" << endl << endl;
}

vtkSmartPointer<vtkPolyData> VTKMeshData::importObjFile( const std::string& pathToObjFile )
{
    cout << "Load obj file " << pathToObjFile << endl ;

    vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName( pathToObjFile.c_str() );
    if( m_progressCallback.Get() != NULL )
    {
        reader->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    reader->Update();

    vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
    mesh->DeepCopy( reader->GetOutput() );

    cout << endl << endl;
    return mesh;
}

void VTKMeshData::exportAsStlFile(const vtkSmartPointer<vtkPolyData>& mesh, const string& path , bool useBinaryExport)
{
    cout << "Mesh export as stl file: " << path << endl;
    vtkSmartPointer<vtkSTLWriter> writer = vtkSTLWriter::New();
    writer->SetFileName( path.c_str() );
    writer->SetInputData( mesh );
    if( useBinaryExport )
    {
        writer->SetFileTypeToBinary();
    }
    else
    {
        writer->SetFileTypeToASCII();
    }
    if( m_progressCallback.Get() != NULL )
    {
        writer->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    writer->Write();
    cout << endl << endl;
}

vtkSmartPointer<vtkPolyData> VTKMeshData::importStlFile( const std::string& pathToStlFile )
{
    cout << "Load stl file " << pathToStlFile << endl;

    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName( pathToStlFile.c_str() );
    if( m_progressCallback.Get() != NULL )
    {
        reader->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    reader->Update();

    vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
    mesh->DeepCopy( reader->GetOutput() );

    cout << endl << endl;
    return mesh;
}

vtkSmartPointer<vtkPolyData> VTKMeshData::importPlyFile( const std::string& pathToPlyFile )
{
    cout << "Load ply file " << pathToPlyFile << endl;

    vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName( pathToPlyFile.c_str() );
    if( m_progressCallback.Get() != NULL )
    {
        reader->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    reader->Update();

    vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
    mesh->DeepCopy( reader->GetOutput() );

    cout << endl << endl;
    return mesh;
}

void VTKMeshData::exportAsPlyFile( const vtkSmartPointer<vtkPolyData>& mesh, const string& path )
{
    cout << "Mesh export as ply file: " << path << endl;
    vtkSmartPointer<vtkPLYWriter> writer = vtkPLYWriter::New();
    writer->SetFileName( path.c_str() );
    writer->SetInputData( mesh );
    writer->SetFileTypeToASCII();
    if( m_progressCallback.Get() != NULL )
    {
        writer->AddObserver(vtkCommand::ProgressEvent, m_progressCallback);
    }
    writer->Write();
    cout << endl << endl;
}
