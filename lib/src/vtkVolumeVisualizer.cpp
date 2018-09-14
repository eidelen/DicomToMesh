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
#include <vtkProperty.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>

// Code based on examples from:
// https://www.vtk.org/Wiki/VTK/Examples/Cxx/VolumeRendering/SmartVolumeMapper
// https://www.vtk.org/Wiki/VTK/Examples/Cxx/Medical/MedicalDemo4
void VTKVolumeVisualizer::displayVolume( const vtkSmartPointer<vtkImageData>& imageData, std::vector<VolumeRenderingColoringEntry> colors )
{
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    // standard interaction
    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renderWindow);

    // setup volume rendering
    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
    volumeMapper->SetInputData(imageData);

    // transfer function
    if( colors.empty() )
    {
        // enable default coloring
        colors.push_back( VolumeRenderingColoringEntry(200,10,10,0,-100) );
        colors.push_back( VolumeRenderingColoringEntry(200,10,10,10,-99) );
        colors.push_back( VolumeRenderingColoringEntry(200,10,10,10,10) );
        colors.push_back( VolumeRenderingColoringEntry(50,10,180,70,11) );
        colors.push_back( VolumeRenderingColoringEntry(20,10,240,100,500) );
        colors.push_back( VolumeRenderingColoringEntry(160,160,160,60,501));
        colors.push_back( VolumeRenderingColoringEntry(255,255,255,90,1500));
    }

    vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
    vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    for( VolumeRenderingColoringEntry e : colors )
    {
        volumeColor->AddRGBPoint( e.m_voxelValue, e.m_red/255.0, e.m_green/255.0, e.m_blue/255.0 );
        volumeScalarOpacity->AddPoint(e.m_voxelValue, e.m_alpha/255.0);
    }

    // The gradient opacity function is used to decrease the opacity
    // in the "flat" regions of the volume while maintaining the opacity
    // at the boundaries between tissue types.
    vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    volumeGradientOpacity->AddPoint(0,   0.0);
    volumeGradientOpacity->AddPoint(90,  0.5);
    volumeGradientOpacity->AddPoint(100, 1.0);

    vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetColor(volumeColor);
    volumeProperty->SetScalarOpacity(volumeScalarOpacity);
    volumeProperty->SetGradientOpacity(volumeGradientOpacity);
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->ShadeOn();
    volumeProperty->SetAmbient(0.3);
    volumeProperty->SetDiffuse(0.8);
    volumeProperty->SetSpecular(1.0);


    vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    renderer->AddViewProp(volume);

    // Set a background color for the renderer
    renderer->SetBackground(0.0, 0.0, 0.0);

    renderWindow->SetSize(1000, 800);

    // Interact with the data.
    iren->Initialize();
    iren->Start();
}



