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


#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vtkAlgorithm.h>

#include "vtkMeshRoutines.h"
#include "vtkDicomRoutines.h"
#include "vtkMeshVisualizer.h"

// Note: In order to safe memory, smart-pointers were not used for certain
//       objects. This has the advantage that memory blocks can be released
//       within the function body.

using namespace std;

struct Dicom2MeshSettings
{
    string pathToInputData;
    bool pathToDicomSet = false;
    bool enabledExportMeshFile = false;
    bool setOriginToCenterOfMass = false;
    bool enableMeshReduction = false;
    bool enablePolygonLimitation = false;
    bool extracOnlyBigObjects = false;
    bool enableSmoothing = false;
    bool showIn3DView = false;
    bool enableCrop = false;
    char pad[3];
    int isoValue = 400; // Hard Tissue
    unsigned long polygonLimit = 100000;
    double nbrVerticesRatio = 0.1;
    double reductionRate = 0.5;
    string outputFilePath = "mesh.stl";
};

void myVtkProgressCallback(vtkObject* caller, long unsigned int /*eventId*/, void* clientData, void* /*callData*/)
{
    // display progress in terminal
    vtkAlgorithm* filter = static_cast<vtkAlgorithm*>(caller);
    char* task = static_cast<char*>(clientData);
    cout << "\33[2K\r"; // erase line
    cout << task << ": ";
    if( filter->GetProgress() > 0.999 )
        cout << "done";
    else
        cout << std::fixed << std::setprecision( 1 )  << filter->GetProgress() * 100 << "%";
    cout << flush;
}

vtkSmartPointer<vtkPolyData> loadInputData( const Dicom2MeshSettings& settings, vtkSmartPointer<vtkCallbackCommand> progressCallback, bool& successful )
{
    vtkSmartPointer<vtkPolyData> mesh;
    bool loadObj = false; bool loadStl = false;

    // check if a mesh file is loaded
    string::size_type idx = settings.pathToInputData.rfind('.');
    if( idx != string::npos )
    {
        string extension = settings.pathToInputData.substr(idx+1);
        loadObj = extension == "obj";
        loadStl = extension == "stl";
    }

    std::shared_ptr<VTKMeshRoutines> vmr = std::shared_ptr<VTKMeshRoutines>( new VTKMeshRoutines() );
    vmr->SetProgressCallback( progressCallback );

    if( loadObj )
    {
        mesh =  vmr->importObjFile( settings.pathToInputData );
        successful = true;
    }
    else if( loadStl )
    {
        mesh = vmr->importStlFile( settings.pathToInputData );
        successful = true;
    }
    else
    {
        std::shared_ptr<VTKDicomRoutines> vdr = std::shared_ptr<VTKDicomRoutines>( new VTKDicomRoutines() );
        vdr->SetProgressCallback( progressCallback );


        vtkSmartPointer<vtkImageData> imgData = vdr->loadDicomImage( settings.pathToInputData );
        if( imgData == NULL )
        {
            cerr << "No image data could be created. Maybe wrong directory?" << endl;
            successful = false;
        }
        else
        {
            if( settings.enableCrop )
                vdr->cropDicom( imgData );

            mesh = vdr->dicomToMesh( imgData, settings.isoValue );
            successful = true;
        }
    }

    return mesh;
}

void showUsage()
{
    cout << "How to use dicom2Mesh:" << endl << endl;

    cout << "Minimum example. This transforms a dicom data set into a 3d mesh file called mesh.stl by using an iso value of 400 (makes bone visible)" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory -o mesh.stl" << endl << endl;

    cout << "This creates a mesh file called abc.obj by using an iso value of 700" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -o abc.obj  -t 700 " << endl << endl;

    cout << "This option offers the possibility to crop the input dicom volume" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -z" << endl << endl;

    cout << "This creates a mesh with a reduced number of polygons by half" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -r" << endl << endl;

    cout << "This creates a mesh with a reduced number of polygons by 80%" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -r 0.8" << endl << endl;

    cout << "This creates a mesh with a limited number of polygons of 10000. This has the same effect as reducing -r the mesh. It does not make sense to use these two options together." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -p 10000" << endl << endl;

    cout << "This creates a mesh where small connected objects are removed. In particular, only connected objects with a minimum number of vertices of 20% of the object with the most vertices are part of the result." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -e  0.2" << endl << endl;

    cout << "This creates a mesh which is shifted to the coordinate system origin." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -c" << endl << endl;

    cout << "This creates a mesh which is smoothed." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -s" << endl << endl;

    cout << "This creates a mesh and shows it in a 3d view." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -v" << endl << endl;

    cout << "Alternatively a mesh file (obj, stl) can be loaded directly. This is handy to modify an existing mesh." << endl;
    cout << "> dicom2mesh -i abc.obj " << endl << endl;

    cout << "Arguments can be combined." << endl << endl;
}

string getSettingsAsString( const Dicom2MeshSettings& settings )
{
    string ret = "Dicom2Mesh Settings\n-------------------\n";
    ret.append("Input directory: "); ret.append(settings.pathToInputData); ret.append("\n");
    ret.append("Output file path: "); ret.append(settings.outputFilePath); ret.append("\n");
    ret.append("Surface segementation: "); ret.append( to_string(settings.isoValue )); ret.append("\n");
    ret.append("Mesh reduction: ");
    if(settings.enableMeshReduction)
    {
        ret.append("enabled (rate="); ret.append( to_string(settings.reductionRate )); ret.append(")\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh polygon limitation: ");
    if(settings.enablePolygonLimitation)
    {
        ret.append("enabled (nbr="); ret.append( to_string(settings.polygonLimit )); ret.append(")\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh smoothing: ");
    if(settings.enableSmoothing)
    {
        ret.append("enabled\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh centering: ");
    if(settings.setOriginToCenterOfMass)
    {
        ret.append("enabled\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh filtering: ");
    if(settings.extracOnlyBigObjects)
    {
        ret.append("enabled (size-ratio="); ret.append( to_string(settings.nbrVerticesRatio )); ret.append(")\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Volume cropping: ");
    if(settings.enableCrop)
    {
        ret.append("enabled\n");
    }
    else
    {
        ret.append("disabled\n");
    }

    return ret;
}

bool parseSettings( const int& argc, char* argv[], Dicom2MeshSettings& settings )
{
    for( int a = 0; a < argc; a++ )
    {
        string cArg( argv[a] );

        if( cArg.compare("-i") == 0 )
        {
            // next argument is path to dicom directory
            a++;
            if( a < argc )
            {
                settings.pathToInputData = argv[a];
                settings.pathToDicomSet = true;
            }
            else
            {
                showUsage();
                return false;
            }
        }
        else if( cArg.compare("-o") == 0 )
        {
            // next argument is file path to mesh output
            a++;
            if( a < argc )
            {
                settings.outputFilePath = argv[a];
                settings.enabledExportMeshFile = true;
            }
            else
            {
                showUsage();
                return false;
            }
        }
        else if( cArg.compare("-t") == 0 )
        {
            // next argument is iso value (int)
            a++;
            if( a < argc )
            {
                settings.isoValue = stoi( string(argv[a]) );
            }
            else
            {
                showUsage();
                return false;
            }
        }
        else if( cArg.compare("-h") == 0 )
        {
            showUsage();
            return false;
        }
        else if( cArg.compare("-r") == 0 )
        {
            settings.enableMeshReduction = true;
            // next argument is reduction (float)
            a++;
            if( a < argc ) // default value is 0.5
                settings.reductionRate = stod( string(argv[a]) );
        }
        else if( cArg.compare("-p") == 0 )
        {
            settings.enablePolygonLimitation = true;
            // next argument is polygon limit
            a++;
            if( a < argc ) // default value is 100000
                settings.polygonLimit = stoul( string(argv[a]) );
        }
        else if( cArg.compare("-e") == 0 )
        {
            settings.extracOnlyBigObjects = true;
            // next argument is size ratio (float)
            a++;
            if( a < argc ) // default value is 0.1
                settings.nbrVerticesRatio = stod( string(argv[a]) );
        }
        else if( cArg.compare("-c") == 0 )
        {
            settings.setOriginToCenterOfMass = true;
        }
        else if( cArg.compare("-s") == 0 )
        {
            settings.enableSmoothing = true;
        }
        else if( cArg.compare("-v") == 0 )
        {
            settings.showIn3DView = true;
        }
        else if( cArg.compare("-z") == 0 )
        {
            settings.enableCrop = true;
        }
    }

    if( !settings.pathToDicomSet )
    {
        cerr << "Path to DICOM directory missing" << endl << "> dicom2mesh -i pathToDicom" << endl;
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    //****** Parse arguments *******//
    Dicom2MeshSettings settings;
    if( !parseSettings( argc, argv, settings) )
        return -1;

    cout << endl << getSettingsAsString( settings ) << endl;
    //******************************//

    vtkSmartPointer<vtkCallbackCommand> progressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    progressCallback->SetCallback(myVtkProgressCallback);

    //******** Read DICOM *********//
    bool loadSuccessful;
    vtkSmartPointer<vtkPolyData> mesh = loadInputData( settings, progressCallback, loadSuccessful );
    if( !loadSuccessful )
        return -1;
    //******************************//

    if( mesh->GetNumberOfCells() == 0 )
    {
        cerr << "No mesh could be created. Wrong DICOM or wrong iso value" << endl;
        return -1;
    }

    //***** Mesh post-processing *****//
    std::shared_ptr<VTKMeshRoutines> vmr = std::shared_ptr<VTKMeshRoutines>( new VTKMeshRoutines() );
    vmr->SetProgressCallback( progressCallback );

    if( settings.setOriginToCenterOfMass )
        vmr->moveMeshToCOSCenter( mesh );

    if( settings.enableMeshReduction )
    {
        // check reduction rate
        if( settings.reductionRate < 0.0 || settings.reductionRate > 1.0 )
            cout << "Reduction skipped due to invalid reductionRate " << settings.reductionRate << " where a value of 0.0 - 1.0 is expected." << endl;
        else
           vmr->meshReduction( mesh, settings.reductionRate );
    }

    if( settings.enablePolygonLimitation )
    {
        if( mesh->GetNumberOfCells() > vtkIdType(settings.polygonLimit) )
        {
            double reductionRate = 1.0 - (  (double(settings.polygonLimit)) / (double(mesh->GetNumberOfCells()))) ;
            vmr->meshReduction( mesh, reductionRate );
        }
        else
        {
            cout << "Reducing polygons not necessary." << endl << endl;
        }
    }

    if( settings.extracOnlyBigObjects )
    {
        if( settings.nbrVerticesRatio < 0.0 || settings.nbrVerticesRatio > 1.0 )
            cout << "Smoothing skipped due to invalid reductionRate " << settings.nbrVerticesRatio << " where a value of 0.0 - 1.0 is expected." << endl;
        else
            vmr->removeSmallObjects( mesh, settings.nbrVerticesRatio );
    }

    if( settings.enableSmoothing )
    {
        vmr->smoothMesh( mesh, 20 );
    }

    //********************************//

    // check if obj or stl
    if( settings.enabledExportMeshFile )
    {
        string::size_type idx = settings.outputFilePath.rfind('.');
        if( idx != string::npos )
        {
            string extension = settings.outputFilePath.substr(idx+1);

            if( extension == "obj" )
                vmr->exportAsObjFile( mesh, settings.outputFilePath );
            else if( extension == "stl" )
                vmr->exportAsStlFile( mesh, settings.outputFilePath );
            else
                cerr << "Unknown file type" << endl;
        }
        else
        {
            cerr << "No Filename." << endl;
        }
    }

    if( settings.showIn3DView )
        VTKMeshVisualizer::displayMesh( mesh );

    return 0;
}



