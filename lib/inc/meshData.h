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

#ifndef _vtkMeshData_H_
#define _vtkMeshData_H_

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkVector.h>
#include <vtkCallbackCommand.h>
#include <string>
#include <vector>

class VTKMeshData
{

public:

    VTKMeshData();
    ~VTKMeshData();

    /**
     * Sets the progress callback function.
     * @param progressCallback A progress callback.
     */
    void SetProgressCallback( vtkSmartPointer<vtkCallbackCommand> progressCallback );

    /**
     * Export the mesh in STL format.
     * @param mesh Mesh to export.
     * @param path Path to the exported stl file.
     * @param useBinaryExport Defines if export is written in a binary format
     */
    void exportAsStlFile( const vtkSmartPointer<vtkPolyData>& mesh, const std::string& path, bool useBinaryExport = false );

    /**
     * Export the mesh in OBJ format.
     * @param mesh Mesh to export.
     * @param path Path to the exported obj file.
     */
    void exportAsObjFile( const vtkSmartPointer<vtkPolyData>& mesh, const std::string& path );

    /**
     * Opens a obj file and returns a vtkPolyData mesh.
     * @param pathToObjFile Path to the obj file.
     * @return Resulting 3D mesh.
     */
    vtkSmartPointer<vtkPolyData> importObjFile( const std::string& pathToObjFile );

    /**
     * Opens a stl file and returns a vtkPolyData mesh.
     * @param pathToStlFile Path to the stl file.
     * @return Resulting 3D mesh.
     */
    vtkSmartPointer<vtkPolyData> importStlFile( const std::string& pathToStlFile );

    /**
     * Opens a ply file and returns a vtkPolyData mesh.
     * @param pathToPlyFile Path to the ply file.
     * @return Resulting 3D mesh.
     */
    vtkSmartPointer<vtkPolyData> importPlyFile( const std::string& pathToPlyFile );

    /**
     * Export the mesh in PLY format.
     * @param mesh Mesh to export.
     * @param path Path to the exported ply file.
     */
    void exportAsPlyFile( const vtkSmartPointer<vtkPolyData>& mesh, const std::string& path );


    /**
     * Compute the vertex normals of a mesh.
     * @param mesh The mesh, of which the vertex nomrals are computed.
     * @param normals Contains normals at return.
     */
    static void computeVertexNormalsTrivial( const vtkSmartPointer<vtkPolyData>& mesh, std::vector<vtkVector3d>& normals );


private:

    vtkSmartPointer<vtkCallbackCommand> m_progressCallback;
};

#endif // _vtkMeshData_H_
