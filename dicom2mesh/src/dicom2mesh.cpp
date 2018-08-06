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

#include "dicom2mesh.h"
#include "vtkMeshRoutines.h"
#include "vtkDicomRoutines.h"
#include "vtkMeshVisualizer.h"

#include <vtkAlgorithm.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <chrono>

// Note: In order to safe memory, smart-pointers were not used for certain
//       objects. This has the advantage that memory blocks can be released
//       within the function body.

void myVtkProgressCallback(vtkObject* caller, long unsigned int /*eventId*/, void* /*clientData*/, void* /*callData*/)
{
    // display progress in terminal
    vtkAlgorithm* filter = static_cast<vtkAlgorithm*>(caller);
    std::cout << "\33[2K\r"; // erase line
    std::cout << "Progress: ";
    if( filter->GetProgress() > 0.999 )
        std::cout << "done";
    else
        std::cout << std::fixed << std::setprecision( 1 )  << filter->GetProgress() * 100 << "%";
    std::cout << std::flush;
}

Dicom2Mesh::Dicom2Mesh(const Dicom2MeshParameters& params)
{
    m_params = params;

    m_vtkCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    m_vtkCallback->SetCallback(myVtkProgressCallback);
}

Dicom2Mesh::~Dicom2Mesh()
{

}

int Dicom2Mesh::doMesh()
{
    std::chrono::steady_clock::time_point t_begin = std::chrono::steady_clock::now();

    std::cout << std::endl << getParametersAsString(m_params) << std::endl;
    //******************************//

    //******** Read DICOM *********//
    bool loadSuccessful;
    vtkSmartPointer<vtkPolyData> mesh = loadInputData( loadSuccessful );
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
    vmr->SetProgressCallback( m_vtkCallback );

    if( m_params.setOriginToCenterOfMass )
    {
        vtkVector3d trans = vmr->moveMeshToCOSCenter(mesh);
        cout << "Move mesh to the coordinate systems's center: Translation [" << trans.GetX() << "," << trans.GetY() << "," << trans.GetZ() << "]" << endl << endl;
    }

    if( m_params.enableMeshReduction )
    {
        // check reduction rate
        if( m_params.reductionRate < 0.0 || m_params.reductionRate > 1.0 )
            std::cout << "Reduction skipped due to invalid reduction rate " << m_params.reductionRate << " where a value of 0.0 - 1.0 is expected." << endl;
        else
            vmr->meshReduction( mesh, m_params.reductionRate );
    }

    if( m_params.enablePolygonLimitation )
    {
        if( mesh->GetNumberOfCells() > vtkIdType(m_params.polygonLimit) )
        {
            double reductionRate = 1.0 - (  (double(m_params.polygonLimit)) / (double(mesh->GetNumberOfCells()))) ;
            vmr->meshReduction( mesh, reductionRate );
        }
        else
        {
            std::cout << "Reducing polygons not necessary." << endl << endl;
        }
    }

    if( m_params.extracOnlyBigObjects )
    {
        if( m_params.nbrVerticesRatio < 0.0 || m_params.nbrVerticesRatio > 1.0 )
            std::cout << "Filtering skipped due to invalid filter rate " << m_params.nbrVerticesRatio << " where a value of 0.0 - 1.0 is expected." << endl;
        else
            vmr->removeSmallObjects( mesh, m_params.nbrVerticesRatio );
    }

    if( m_params.enableSmoothing )
    {
        vmr->smoothMesh( mesh, 20 );
    }

    //********************************//

    // check if obj or stl
    if( m_params.enabledExportMeshFile )
    {
        std::string::size_type idx = m_params.outputFilePath.rfind('.');
        if( idx != std::string::npos )
        {
            std::string extension = m_params.outputFilePath.substr(idx+1);

            if( extension == "obj" )
                vmr->exportAsObjFile( mesh, m_params.outputFilePath );
            else if( extension == "stl" )
                vmr->exportAsStlFile( mesh, m_params.outputFilePath );
            else if( extension == "ply" )
                vmr->exportAsPlyFile( mesh, m_params.outputFilePath );
            else
                cerr << "Unknown file type" << endl;


            // safe mesh parameters in info file
            std::string infoFilePath = m_params.outputFilePath.substr(0,idx+1).append("info");
            std::ofstream infoFile;
            infoFile.open(infoFilePath);
            infoFile << getParametersAsString(m_params);
            infoFile.close();
            std::cout << "Parameters written to file:  " << infoFilePath << std::endl;
        }
        else
        {
            cerr << "No Filename." << endl;
        }
    }

    std::chrono::steady_clock::time_point t_done = std::chrono::steady_clock::now();
    std::cout << std::endl << "Required computing time: " << std::chrono::duration_cast<std::chrono::seconds>(t_done - t_begin).count() << " seconds" << std::endl;

    if( m_params.showIn3DView )
        VTKMeshVisualizer::displayMesh( mesh );

    return 0;
}

bool Dicom2Mesh::parseCmdLineParameters(const int &argc, char **argv, Dicom2MeshParameters &param)
{
    for( int a = 0; a < argc; a++ )
    {
        std::string cArg( argv[a] );

        if( cArg.compare("-i") == 0 )
        {
            // next argument is path to dicom directory
            a++;
            if( a < argc )
            {
                param.pathToInputData = argv[a];
                param.pathToDicomSet = true;
            }
            else
            {
                showUsageText();
                return false;
            }
        }
        else if( cArg.compare("-o") == 0 )
        {
            // next argument is file path to mesh output
            a++;
            if( a < argc )
            {
                param.outputFilePath = argv[a];
                param.enabledExportMeshFile = true;
            }
            else
            {
                showUsageText();
                return false;
            }
        }
        else if( cArg.compare("-t") == 0 )
        {
            // next argument is iso value (int)
            a++;
            if( a < argc )
            {
                param.isoValue = std::stoi( std::string(argv[a]) );
            }
            else
            {
                showUsageText();
                return false;
            }
        }
        else if( cArg.compare("-h") == 0 )
        {
            showUsageText();
            return false;
        }
        else if( cArg.compare("-r") == 0 )
        {
            param.enableMeshReduction = true;
            // next argument is reduction (float)
            a++;
            if( a < argc ) // default value is 0.5
                param.reductionRate = std::stod( std::string(argv[a]) );
        }
        else if( cArg.compare("-p") == 0 )
        {
            param.enablePolygonLimitation = true;
            // next argument is polygon limit
            a++;
            if( a < argc ) // default value is 100000
                param.polygonLimit = std::stoul( std::string(argv[a]) );
        }
        else if( cArg.compare("-e") == 0 )
        {
            param.extracOnlyBigObjects = true;
            // next argument is size ratio (float)
            a++;
            if( a < argc ) // default value is 0.1
                param.nbrVerticesRatio = std::stod( std::string(argv[a]) );
        }
        else if( cArg.compare("-c") == 0 )
        {
            param.setOriginToCenterOfMass = true;
        }
        else if( cArg.compare("-s") == 0 )
        {
            param.enableSmoothing = true;
        }
        else if( cArg.compare("-v") == 0 )
        {
            param.showIn3DView = true;
        }
        else if( cArg.compare("-z") == 0 )
        {
            param.enableCrop = true;
        }
        else if( cArg.compare("--version") == 0 )
        {
            showVersionText();
            return false;
        }
    }

    if( !param.pathToDicomSet )
    {
        cerr << "Path to DICOM directory missing" << endl << "> dicom2mesh -i pathToDicom" << endl;
        cerr << "For help, run" << endl << "> dicom2mesh -h" << endl;
        return false;
    }

    return true;
}

void Dicom2Mesh::showUsageText()
{
    std::cout << "How to use dicom2Mesh:" << std::endl << std::endl;

    std::cout << "Minimum example. This transforms a dicom data set into a 3d mesh file called mesh.stl by using a default iso value of 400 (shows bone)" << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory -o mesh.stl" << std::endl << std::endl;

    std::cout << "This creates a mesh file called abc.obj by using a custom iso value of 700" << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -o abc.obj  -t 700 " << std::endl << std::endl;

    std::cout << "This option offers the possibility to crop the input dicom volume. The created mesh is called def.ply." << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -z  -o def.ply" << std::endl << std::endl;

    std::cout << "This creates a mesh with a reduced number of polygons by 50% as default" << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -r" << std::endl << std::endl;

    std::cout << "This creates a mesh with a reduced number of polygons by 80%" << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -r 0.8" << std::endl << std::endl;

    std::cout << "This creates a mesh with a limited number of polygons of 10000. This has the same effect as reducing -r the mesh. It does not make sense to use these two options together." << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -p 10000" << std::endl << std::endl;

    std::cout << "This creates a mesh where small connected objects are removed. In particular, only connected objects with a minimum number of vertices of 20% of the object with the most vertices are part of the result." << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -e  0.2" << std::endl << std::endl;

    std::cout << "This creates a mesh which is shifted to the coordinate system origin." << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -c" << std::endl << std::endl;

    std::cout << "This creates a mesh which is smoothed." << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -s" << std::endl << std::endl;

    std::cout << "This creates a mesh and shows it in a 3d view." << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -v" << std::endl << std::endl;

    std::cout << "Alternatively a mesh file (obj, stl, ply) can be loaded directly, modified and exported again. This is handy to modify an existing mesh. Following example centers and saves a mesh as cba.stl." << std::endl;
    std::cout << "> dicom2mesh -i abc.obj -c -o cba.stl " << std::endl << std::endl;

    std::cout << "Arguments can be combined." << std::endl << std::endl;
}

void Dicom2Mesh::showVersionText()
{
    std::cout << "dicom2Mesh version 0.7.0,   AOT AG - Switzerland" << std::endl;
}


vtkSmartPointer<vtkPolyData> Dicom2Mesh::loadInputData( bool& successful )
{
    vtkSmartPointer<vtkPolyData> mesh;
    bool loadObj = false; bool loadStl = false; bool loadPly = false;

    // check if input is a mesh file
    std::string::size_type idx = m_params.pathToInputData.rfind('.');
    if( idx != std::string::npos )
    {
        std::string extension = m_params.pathToInputData.substr(idx+1);
        loadObj = extension == "obj";
        loadStl = extension == "stl";
        loadPly = extension == "ply";
    }

    std::shared_ptr<VTKMeshRoutines> vmr = std::shared_ptr<VTKMeshRoutines>( new VTKMeshRoutines() );
    vmr->SetProgressCallback( m_vtkCallback );

    if( loadObj )
    {
        mesh =  vmr->importObjFile( m_params.pathToInputData );
        successful = true;
    }
    else if( loadStl )
    {
        mesh = vmr->importStlFile( m_params.pathToInputData );
        successful = true;
    }
    else if( loadPly )
    {
        mesh = vmr->importPlyFile( m_params.pathToInputData );
        successful = true;
    }
    else
    {
        // if not a mesh file, it is a dicom directroy

        std::shared_ptr<VTKDicomRoutines> vdr = std::shared_ptr<VTKDicomRoutines>( new VTKDicomRoutines() );
        vdr->SetProgressCallback( m_vtkCallback );


        vtkSmartPointer<vtkImageData> imgData = vdr->loadDicomImage( m_params.pathToInputData );
        if( imgData == NULL )
        {
            cerr << "No image data could be created. Maybe wrong directory?" << endl;
            successful = false;
        }
        else
        {
            if( m_params.enableCrop )
                vdr->cropDicom( imgData );

            mesh = vdr->dicomToMesh( imgData, m_params.isoValue );
            successful = true;
        }
    }

    return mesh;
}

std::string Dicom2Mesh::getParametersAsString(const Dicom2MeshParameters& params)
{
    std::string ret = "Dicom2Mesh Settings\n-------------------\n";
    ret.append("Input directory: "); ret.append(params.pathToInputData); ret.append("\n");
    ret.append("Output file path: "); ret.append(params.outputFilePath); ret.append("\n");
    ret.append("Surface segmentation: "); ret.append( std::to_string(params.isoValue )); ret.append("\n");
    ret.append("Mesh reduction: ");
    if(params.enableMeshReduction)
    {
        ret.append("enabled (rate="); ret.append( std::to_string(params.reductionRate )); ret.append(")\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh polygon limitation: ");
    if(params.enablePolygonLimitation)
    {
        ret.append("enabled (nbr="); ret.append( std::to_string(params.polygonLimit )); ret.append(")\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh smoothing: ");
    if(params.enableSmoothing)
    {
        ret.append("enabled\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh centering: ");
    if(params.setOriginToCenterOfMass)
    {
        ret.append("enabled\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh filtering: ");
    if(params.extracOnlyBigObjects)
    {
        ret.append("enabled (size-ratio="); ret.append( std::to_string(params.nbrVerticesRatio )); ret.append(")\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Volume cropping: ");
    if(params.enableCrop)
    {
        ret.append("enabled\n");
    }
    else
    {
        ret.append("disabled\n");

    }

    return ret;
}
