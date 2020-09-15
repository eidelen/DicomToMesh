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
#include "meshRoutines.h"
#include "meshData.h"
#include "dicomFactory.h"
#include "dicomRoutines.h"

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
    std::unique_ptr<VTKMeshData> vmd = std::unique_ptr<VTKMeshData>( new VTKMeshData() );
    vmd->SetProgressCallback( m_vtkCallback );
    std::unique_ptr<VTKMeshRoutines> vmr = std::unique_ptr<VTKMeshRoutines>( new VTKMeshRoutines() );
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
                vmd->exportAsObjFile( mesh, m_params.outputFilePath );
            else if( extension == "stl" )
                vmd->exportAsStlFile( mesh, m_params.outputFilePath, m_params.useBinaryExport );
            else if( extension == "ply" )
                vmd->exportAsPlyFile( mesh, m_params.outputFilePath );
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

    return 0;
}

// from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
std::string Dicom2Mesh::trim(const std::string& str)
{
    std::string trimedStr = str;
    trimedStr.erase(0, trimedStr.find_first_not_of(' '));       //prefixing spaces
    trimedStr.erase(trimedStr.find_last_not_of(' ')+1);         //surfixing spaces
    return trimedStr;
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
        if( cArg.compare("-ipng") == 0 )
        {
            param.inputAsPngFileList = true;
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
        else if( cArg.compare("-b") == 0 )
        {
            param.useBinaryExport = true;
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

        }
        else if( cArg.at(0) == '[' )
        {
            // Concatenate multiple string arguments till next ]
            std::string filesText = "";
            bool goOn = true;
            do
            {
                std::string part(argv[a]);
                filesText.append(part);

                if( filesText.back() == ']' ||  a >= argc)
                    goOn = false;
                else
                    a++;
            }
            while( goOn );

            // Extract [ content ]
            size_t startPos = filesText.find('[');
            size_t endPos = filesText.find(']');
            std::string extFiles = filesText.substr(startPos+1, endPos - startPos - 1);

            std::vector<std::string> fileList = parseCommaSeparatedStr(extFiles);
            if(fileList.size() > 0)
            {
                param.inputImageFiles = fileList;
            }
            else
            {
                std::cerr << "No image files specified" << std::endl;
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
        else if( cArg.compare("-sxyz") == 0 )
        {
            // next three arguments are spacings
            if( (a+3) < argc)
            {
                param.x_spacing = std::stod(std::string(argv[++a]));
                param.y_spacing = std::stod(std::string(argv[++a]));
                param.z_spacing = std::stod(std::string(argv[++a]));
            }
        }
    }

    if( !param.pathToInputAvailable && !param.inputAsPngFileList )
    {
        cerr << "Path to Dicom directory missing" << endl << "> dicom2mesh -i pathToDicom" << endl;
        cerr << "or" << endl;
        cerr << "No input images defined" << endl << "> dicom2mesh -ipng [path1, path2, ...]" << endl;
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

    std::cout << "This creates a mesh file in a binary format called mesh.stl" << std::endl;
    std::cout << "> dicom2mesh -i pathToDicomDirectory -b -o mesh.stl" << std::endl << std::endl;

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

    std::cout << "A mesh can be created based on a list of png-file slices as input. The three floats followed after -sxyz are the x/y/z-spacing." << std::endl;
    std::cout << "> dicom2mesh -ipng [path1, path2, path3, ...] -sxyz 1.5 1.5 3.0  -c -o cba.stl " << std::endl << std::endl;

    std::cout << "Arguments can be combined." << std::endl << std::endl;
}

void Dicom2Mesh::showVersionText()
{
    std::cout << "dicom2Mesh version 0.8.0,   AOT AG - Switzerland" << std::endl;
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

    std::shared_ptr<VTKMeshData> vmd = std::shared_ptr<VTKMeshData>( new VTKMeshData() );
    vmd->SetProgressCallback( m_vtkCallback );

    if( loadObj )
    {
        mesh3d =  vmd->importObjFile( m_params.pathToInputData );
        result = true;
    }
    else if( loadStl )
    {
        mesh3d = vmd->importStlFile( m_params.pathToInputData );
        result = true;
    }
    else if( loadPly )
    {
        mesh3d = vmd->importPlyFile( m_params.pathToInputData );
        result = true;
    }
    else
    {
        // if not a mesh file, it is a directory of images

        std::shared_ptr<VTKDicomRoutines> vdr = VTKDicomFactory::getDicomRoutines();
        vdr->SetProgressCallback( m_vtkCallback );

        if( m_params.inputAsPngFileList )
        {
            // set of png images
            volume = vdr->loadPngImages( m_params.inputImageFiles, m_params.x_spacing, m_params.y_spacing, m_params.z_spacing );
        }
        else
        {
            // a dicom data set
            volume = vdr->loadDicomImage(m_params.pathToInputData);
        }

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

std::vector<std::string> Dicom2Mesh::parseCommaSeparatedStr(const std::string& text)
{
    std::vector<std::string> strs;
    std::string toParse = text;

    bool goOn = true;
    while(goOn)
    {
        size_t nextDel = toParse.find(',');
        if( nextDel == std::string::npos )
        {
            goOn = false;
            strs.push_back(trim(toParse)); // just add the remaining string
        }
        else
        {
            std::string aPath = toParse.substr(0,nextDel);
            strs.push_back(trim(aPath));
            toParse.erase(0, nextDel+1);
        }
    }

    return strs;
}
