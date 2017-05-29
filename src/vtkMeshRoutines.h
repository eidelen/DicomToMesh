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

#ifndef _vtkMeshRoutines_H_
#define _vtkMeshRoutines_H_

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkVector.h>
#include <vtkCallbackCommand.h>
#include <string>
#include <vector>

class VTKMeshRoutines
{

public:

    /**
     * Moves the mesh to center of the coordinate system. In particular,
     * the center of mass is computed and the mesh is translated accordingly.
     * @param mesh The input mesh. Mesh will be modified afterwards.
     */
    static void moveMeshToCOSCenter( vtkSmartPointer<vtkPolyData> mesh );

    /**
     * Reduces the size / details of a 3D mesh.
     * @param mesh The input mesh. Mesh will be modified afterwards.
     * @param reduction Reduction factor. 0.1 is little reduction. 0.9 is strong reduction.
     * @param progressCallback Progress callback function pointer.
     */
    static void meshReduction( vtkSmartPointer<vtkPolyData> mesh, const float& reduction, vtkSmartPointer<vtkCallbackCommand> progressCallback );

    /**
     * Labels connected regions and removes regions below a certain size.
     * @param mesh The input mesh. Mesh will be modified afterwards.
     * @param ratio Value between 0.0 - 1.0. Value of 0.2 indicates that only
     *              object with a minimum number of 20% vertices relative to the
     *              number of vertices in the largest objects are extracted.
     */
    static void removeSmallObjects( vtkSmartPointer<vtkPolyData> mesh, const float& ratio );

    /**
     * Smooths the mesh surface.
     * @param mesh The input mesh. Mesh will be modified afterwards.
     * @param nbrOfSmoothingIterations Number of smoothing iterations.
     * @param progressCallback Progress callback function pointer.
     */
    //Todo: Understand FeatureAngle and RelaxationFactor. Then add it as argument.
    static void smoothMesh( vtkSmartPointer<vtkPolyData> mesh, unsigned int nbrOfSmoothingIterations, vtkSmartPointer<vtkCallbackCommand> progressCallback );

    /**
     * Export the mesh in STL format.
     * @param mesh Mesh to export.
     * @param path Path to the exported stl file.
     */
    static void exportAsStlFile( const vtkSmartPointer<vtkPolyData>& mesh, const std::string& path );

    /**
     * Export the mesh in OBJ format.
     * @param mesh Mesh to export.
     * @param path Path to the exported obj file.
     */
    static void exportAsObjFile( const vtkSmartPointer<vtkPolyData>& mesh, const std::string& path );

    /**
     * Opens a obj file and returns a vtkPolyData mesh.
     * @param pathToObjFile Path to the obj file.
     * @return Resulting 3D mesh.
     */
    static vtkPolyData* importObjFile( const std::string& pathToObjFile );

    static void computeVertexNormalsTrivial( const vtkSmartPointer<vtkPolyData>& mesh, std::vector<vtkVector3d>& normals );
};

#endif // _vtkMeshRoutines_H_
