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

#include "vtkVolumeVisualizer.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkProperty.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>

// Code mainly taken from this example: https://www.vtk.org/Wiki/VTK/Examples/Cxx/VolumeRendering/SmartVolumeMapper

void VTKVolumeVisualizer::displayVolume( const vtkSmartPointer<vtkImageData>& imageData )
{
    // show mesh
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    renderWindow->SetSize(1000, 800);

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    /*vtkSmartPointer<VTKVolumeVisualizerInteraction> pickingInteraction = vtkSmartPointer<VTKVolumeVisualizerInteraction>::New();
    pickingInteraction->SetDefaultRenderer(renderer);
    renderWindowInteractor->SetInteractorStyle( pickingInteraction );*/
    renderWindowInteractor->SetRenderWindow( renderWindow );

//    renderWindow->Render();

    // Visualize
    vtkSmartPointer<vtkSmartVolumeMapper> volM = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    volM->SetBlendModeToComposite(); // composite first


#if VTK_MAJOR_VERSION <= 5
    volM->SetInputConnection(imageData->GetProducerPort());
#else
    volM->SetInputData(imageData);
#endif

    vtkSmartPointer<vtkVolumeProperty> volProp = vtkSmartPointer<vtkVolumeProperty>::New();
    volProp->SetInterpolationTypeToLinear();
    volProp->ShadeOn();
    volProp->SetAmbient(0.4);
    volProp->SetDiffuse(0.6);
    volProp->SetSpecular(0.2);

    // transfer function
    vtkSmartPointer<vtkPiecewiseFunction> compOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    compOpacity->AddPoint(0,    0.00);
    compOpacity->AddPoint(10,   0.10);
    compOpacity->AddPoint(300,  0.25);
    compOpacity->AddPoint(700, 0.8);
    compOpacity->AddPoint(2000, 0.95);
    volProp->SetScalarOpacity(compOpacity); // composite first.
    vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
    color->AddRGBPoint(0,    0.0, 0.0, 0.0);
    color->AddRGBPoint(100,  1.0, 0.0, 0.0);
    color->AddRGBPoint(300,  1.0, 0.0, 0.0);
    color->AddRGBPoint(700,  0.6, 0.6, 0.6);
    color->AddRGBPoint(2000, 1.0, 1.0, 1.0);
    volProp->SetColor(color);

    vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volM);
    volume->SetProperty(volProp);
    renderer->AddViewProp(volume);
    renderer->ResetCamera();

    // Render composite. In default mode. For coverage.
    //renderWindow->Render();

    // 3D texture mode. For coverage.
#if !defined(VTK_LEGACY_REMOVE) && !defined(VTK_OPENGL2)
    volM->SetRequestedRenderModeToRayCastAndTexture();
#endif // VTK_LEGACY_REMOVE
    renderWindow->Render();

    volM->SetRequestedRenderModeToRayCast();
    //renderWindow->Render();

    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();
}



