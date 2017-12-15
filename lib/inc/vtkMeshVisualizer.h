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

#ifndef _vtkMeshVisualizer_H_
#define _vtkMeshVisualizer_H_

#include "dllDefines.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkActor.h>

#include <chrono>

class VTKMeshVisualizerInteraction : public vtkInteractorStyleTrackballCamera
{
public:
    static MYLIB_EXPORT VTKMeshVisualizerInteraction* New();
    vtkTypeMacro( VTKMeshVisualizerInteraction, vtkInteractorStyleTrackballCamera )

    virtual void OnLeftButtonDown() override;

private:
    VTKMeshVisualizerInteraction();
    void initIfNecessary();
    bool isDoubleClick();

private:
    vtkSmartPointer<vtkActor> m_pickingMarker;
    std::chrono::system_clock::time_point m_lastMouseClick;
};



class VTKMeshVisualizer
{

public:
    /**
     * Display mesh in a 3d view.
     * @param mesh Mesh to show.
     */
    static MYLIB_EXPORT void displayMesh( const vtkSmartPointer<vtkPolyData>& mesh );
};


#endif //_vtkMeshVisualizer_H_
