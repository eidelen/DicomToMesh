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
#include "vtkDicomFactory.h"
#include "vtkDicomRoutines.h"
#include "vtkMeshVisualizer.h"
#include "vtkVolumeVisualizer.h"

#include <vtkAlgorithm.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <chrono>
#include <regex>

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
    vtkSmartPointer<vtkPolyData> mesh;
    vtkSmartPointer<vtkImageData> volume;
    if( !loadInputData( volume, mesh) )
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

    if( m_params.enableOriginToCenterOfMass )
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

    if( m_params.enableObjectFiltering )
    {
        if( m_params.objectSizeRatio < 0.0 || m_params.objectSizeRatio > 1.0 )
            std::cout << "Filtering skipped due to invalid filter rate " << m_params.objectSizeRatio << " where a value of 0.0 - 1.0 is expected." << endl;
        else
            vmr->removeSmallObjects( mesh, m_params.objectSizeRatio );
    }

    if( m_params.enableSmoothing )
    {
        vmr->smoothMesh( mesh, 20 );
    }

    //********************************//

    // check if obj or stl
    if( m_params.pathToOutputAvailable )
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

    if( m_params.doVisualize )
    {
        if( m_params.showAsVolume )
        {
            VTKVolumeVisualizer::displayVolume(volume, m_params.volumenRenderingColoring);
        }
        else
        {
            VTKMeshVisualizer::displayMesh(mesh);
        }
    }

    return 0;
}

bool Dicom2Mesh::parseCmdLineParameters(const int &argc, const char **argv, Dicom2MeshParameters &param)
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
                param.pathToInputAvailable = true;
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
                param.pathToOutputAvailable = true;
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
        else if( cArg.compare("-tu") == 0 )
        {
            // next argument is upper iso value (int)
            a++;
            if( a < argc )
            {
                param.upperIsoValue = std::stoi( std::string(argv[a]) );
                param.useUpperIsoValue = true;
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
            param.enableObjectFiltering = true;
            // next argument is size ratio (float)
            a++;
            if( a < argc ) // default value is 0.1
                param.objectSizeRatio = std::stod( std::string(argv[a]) );
        }
        else if( cArg.compare("-c") == 0 )
        {
            param.enableOriginToCenterOfMass = true;
        }
        else if( cArg.compare("-s") == 0 )
        {
            param.enableSmoothing = true;
        }
        else if( cArg.compare("-v") == 0 )
        {
            param.doVisualize = true;
        }
        else if( cArg.compare("-vo") == 0 )
        {
            param.doVisualize = true;
            param.showAsVolume = true;
        }
        else if( cArg.at(0) == '(' )
        {
            // volume rendering color input till next ')'
            std::string vArg = "";
            bool goOn = true;
            do
            {
                std::string part(argv[a]);
                vArg.append(part);

                if( vArg.back() == ')' ||  a >= argc)
                    goOn = false;
                else
                    a++;
            }
            while( goOn );

            VolumeRenderingColoringEntry volE;
            if( parseVolumeRenderingColorEntry(vArg, volE) )
            {
                param.volumenRenderingColoring.push_back(volE);
            }
            else
            {
                std::cerr << "Incomplete volume rendering color entry" << std::endl;
                return false;
            }
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

    if( !param.pathToInputAvailable )
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

    std::cout << "This creates a mesh file by using a iso value range of 500 to 900" << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -o abc.obj  -t 500 -tu 900 " << std::endl << std::endl;

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

    std::cout << "This shows the dicom data in a volume render [ (Red,Green,Blue,Alpha,Iso-Value) ]." << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory  -vo (255,0,0,0,0) (255,255,255,60,700) ..." << std::endl << std::endl;

    std::cout << "Alternatively a mesh file (obj, stl, ply) can be loaded directly, modified and exported again. This is handy to modify an existing mesh. Following example centers and saves a mesh as cba.stl." << std::endl;
    std::cout << "> dicom2mesh -i abc.obj -c -o cba.stl " << std::endl << std::endl;

    std::cout << "Arguments can be combined." << std::endl << std::endl;
}

void Dicom2Mesh::showVersionText()
{
    std::cout << "dicom2Mesh version 0.7.0,   AOT AG - Switzerland" << std::endl;
}


bool Dicom2Mesh::loadInputData( vtkSmartPointer<vtkImageData>& volume, vtkSmartPointer<vtkPolyData>& mesh3d )
{
    bool result = false;

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
        mesh3d =  vmr->importObjFile( m_params.pathToInputData );
        result = true;
    }
    else if( loadStl )
    {
        mesh3d = vmr->importStlFile( m_params.pathToInputData );
        result = true;
    }
    else if( loadPly )
    {
        mesh3d = vmr->importPlyFile( m_params.pathToInputData );
        result = true;
    }
    else
    {
        // if not a mesh file, it is a dicom directroy

        std::shared_ptr<VTKDicomRoutines> vdr = VTKDicomFactory::getDicomRoutines();
        vdr->SetProgressCallback( m_vtkCallback );


        volume = vdr->loadDicomImage( m_params.pathToInputData );
        if( volume == NULL )
        {
            cerr << "No image data could be created. Maybe wrong directory?" << endl;
            result = false;
        }
        else
        {
            if( m_params.enableCrop )
                vdr->cropDicom( volume );

            mesh3d = vdr->dicomToMesh( volume, m_params.isoValue, m_params.useUpperIsoValue, m_params.upperIsoValue );
            result = true;
        }
    }

    return result;
}

std::string Dicom2Mesh::getParametersAsString(const Dicom2MeshParameters& params) const
{
    std::string ret = "Dicom2Mesh Settings\n-------------------\n";
    ret.append("Input directory: "); ret.append(params.pathToInputData); ret.append("\n");
    ret.append("Output file path: "); ret.append(params.outputFilePath); ret.append("\n");

    ret.append("Surface segmentation: ");
    ret.append(std::to_string(params.isoValue));
    if( params.useUpperIsoValue )
    {
        ret.append(" to ");
        ret.append(std::to_string(params.upperIsoValue));
    }
    ret.append("\n");

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
    if(params.enableOriginToCenterOfMass)
    {
        ret.append("enabled\n");
    }
    else
    {
        ret.append("disabled\n");
    }
    ret.append("Mesh filtering: ");
    if(params.enableObjectFiltering)
    {
        ret.append("enabled (size-ratio="); ret.append( std::to_string(params.objectSizeRatio )); ret.append(")\n");
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

bool toColor( const std::string& text, unsigned char& val)
{
    bool successful = true;

    int valInt = std::stoi(text);
    if( valInt >= 0 && valInt < 256 )
    {
        val = static_cast<unsigned  char>(valInt);
        successful = true;
    }
    else
    {
        std::cerr << valInt << " outside color range 0 - 255" << std::endl;
        successful = false;
    }

    return successful;
}

bool Dicom2Mesh::parseVolumeRenderingColorEntry( const std::string& text, VolumeRenderingColoringEntry& colorEntry )
{
    // regex  \([ ]*([+|0-9]{1,})[ ]*,[ ]*([+|0-9]{1,})[ ]*,[ ]*([+|0-9]{1,})[ ]*,[ ]*([+|0-9]{1,})[ ]*,[ ]*([+-|0-9]{1,})[ ]*\)
    // examples: ( +1,200  , 3 ,4 ,5)   (6 , 7,8,9  ,-10 ) (255,255,255,0,0)

    std::regex reg( "\\([ ]*([+|0-9]{1,})[ ]*,[ ]*([+|0-9]{1,})[ ]*,[ ]*([+|0-9]{1,})[ ]*,[ ]*([+|0-9]{1,})[ ]*,[ ]*([+-|0-9]{1,})[ ]*\\)");
    std::smatch parseResult;

    if(! std::regex_match(text,parseResult,reg) )
    {
        std::cerr << "Invalid volume rendering color syntax" << std::endl;
        return false;
    }

    if( parseResult.size() != 6 )
    {
        std::cerr << "Invalid number elements in volume rendering color entry" << std::endl;
        return false;
    }

    if( toColor(parseResult[1], colorEntry.m_red) && toColor(parseResult[2], colorEntry.m_green) && toColor(parseResult[3], colorEntry.m_blue) && toColor(parseResult[4], colorEntry.m_alpha))
    {
        colorEntry.m_voxelValue = std::stoi(parseResult[5]);
        return true;
    }
    else
    {
        return false;
    }
}
