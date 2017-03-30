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

#include <vtkCallbackCommand.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>
#include <vtkCenterOfMass.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkQuadricDecimation.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkSTLWriter.h>

// Note: In order to safe memory, smart-pointers were not used for certain
//       objects. This has the advantage that memory blocks can be released
//       within the function scope.


using namespace std;


struct Dicom2MeshSettings
{
    string pathToDicomDirectory;
    bool pathToDicomSet = false;
    string outputFilePath = "mesh.stl";
    bool pathToOutputSet = false;
    int isoValue = 400; // Hard Tissue
    bool setOriginToCenterOfMass = false;
    bool enableMeshReduction = false;
    float reductionRate = 0.5;
    bool extracOnlyBigObjects = false;
    float nbrVerticesRatio = 0.1;
    bool enableSmoothing = false;
};


void myVtkProgressCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
    vtkAlgorithm* filter = static_cast<vtkAlgorithm*>(caller);
    char* task = static_cast<char*>(clientData);
    cout << task << ": " << std::fixed << std::setprecision( 1 )  << filter->GetProgress() * 100 << "%" << endl;
    cout << flush;
}

/**
 * Moves the mesh to center of the coordinate system. In particular,
 * the center of mass is computed and the mesh is translated accordingly.
 * @param mesh The input mesh. Mesh will be modified afterwards.
 */
void moveMeshToCOSCenter( vtkSmartPointer<vtkPolyData> mesh )
{
    vtkSmartPointer<vtkCenterOfMass> computeCenter = vtkSmartPointer<vtkCenterOfMass>::New();
    computeCenter->SetInputData( mesh );
    computeCenter->SetUseScalarsAsWeights(false);
    computeCenter->Update();

    double objectCenter[3];
    computeCenter->GetCenter(objectCenter);

    cout << "Move origin to center of mass: [" << objectCenter[0] << "," << objectCenter[1] << "," << objectCenter[2] << "]" << endl;

    vtkSmartPointer<vtkTransform> translation = vtkTransform::New();
    translation->Translate(-objectCenter[0], -objectCenter[1], -objectCenter[2]);

    vtkTransformPolyDataFilter* transformFilter = vtkTransformPolyDataFilter::New();
    transformFilter->SetInputData( mesh );
    transformFilter->SetTransform( translation );
    transformFilter->Update();

    mesh->DeepCopy( transformFilter->GetOutput() );

    // Free memory
    transformFilter->Delete();
    cout << endl << "Done" << endl << endl;
}

/**
 * Reduces the size / details of a 3D mesh.
 * @param mesh The input mesh. Mesh will be modified afterwards.
 * @param reduction Reduction factor. 0.1 is little reduction. 0.9 is strong reduction.
 * @param progressCallback Progress callback function pointer.
 */
void meshReduction( vtkSmartPointer<vtkPolyData> mesh, const float& reduction, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    string progressData = "Reduce mesh";
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    unsigned int numberOfCellsBefore = mesh->GetNumberOfCells();
    cout << "Mesh reduction by " << reduction << endl;

    // Note1: vtkQuadricDecimation seems to be better than vtkDecimatePro
    // Note2: vtkQuadricDecimation might have problem with face normals
    vtkQuadricDecimation* decimator = vtkQuadricDecimation::New();
    decimator->SetInputData( mesh );
    decimator->SetTargetReduction( reduction );
    decimator->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    decimator->Update();

    mesh->DeepCopy( decimator->GetOutput() );

    // Free memory
    decimator->Delete();

    unsigned int numberOfCellsAfter = mesh->GetNumberOfCells();
    cout << endl << "Mesh reduced from " << numberOfCellsBefore << " to " <<  numberOfCellsAfter << " faces" << endl;
    cout << "Done" << endl << endl;
}

/**
 * Labels connected regions and removes regions below a certain size.
 * @param mesh The input mesh. Mesh will be modified afterwards.
 * @param ratio Value between 0.0 - 1.0. Value of 0.2 indicates that only
 *              object with a minimum number of 20% vertices relative to the
 *              number of vertices in the largest objects are extracted.
 */
void removeSmallObjects( vtkSmartPointer<vtkPolyData> mesh, const float& ratio )
{
    cout << "Remove small connected objects: Size ratio = " << ratio << endl;

    vtkPolyDataConnectivityFilter* connectivityFilter = vtkPolyDataConnectivityFilter::New();
    connectivityFilter->SetInputData( mesh );
    connectivityFilter->SetExtractionModeToAllRegions();
    connectivityFilter->Update();

    // remove objects consisting of less than ratio vertexes of the biggest object
    vtkIdTypeArray* regionSizes = connectivityFilter->GetRegionSizes();

    // find object with most vertices
    long maxSize = 0;
    for( int regions = 0; regions < connectivityFilter->GetNumberOfExtractedRegions(); regions++ )
        if( regionSizes->GetValue(regions) > maxSize )
            maxSize = regionSizes->GetValue(regions);


    // append regions of sizes over the threshold
    connectivityFilter->SetExtractionModeToSpecifiedRegions();
    for( int regions = 0; regions < connectivityFilter->GetNumberOfExtractedRegions(); regions++ )
        if( regionSizes->GetValue(regions) > maxSize * ratio )
            connectivityFilter->AddSpecifiedRegion(regions);

    connectivityFilter->Update();

    mesh->DeepCopy( connectivityFilter->GetOutput() );

    // Free memory
    connectivityFilter->Delete();

    cout << "Done" << endl << endl;
}

/**
 * Smooths the mesh surface.
 * @param mesh The input mesh. Mesh will be modified afterwards.
 * @param nbrOfSmoothingIterations Number of smoothing iterations.
 * @param progressCallback Progress callback function pointer.
 */
//Todo: Understand FeatureAngle and RelaxationFactor. Then add it as argument.
void smoothMesh( vtkSmartPointer<vtkPolyData> mesh, unsigned int nbrOfSmoothingIterations, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
    string progressData = "Smooth mesh";
    progressCallback->SetClientData( (void*) (progressData.c_str()) );
    cout << "Mesh smoothing with " << nbrOfSmoothingIterations << " iterations." << endl;

    vtkSmoothPolyDataFilter* smoother = vtkSmoothPolyDataFilter::New();
    smoother->SetInputData( mesh );
    smoother->SetNumberOfIterations( nbrOfSmoothingIterations );
    smoother->SetFeatureAngle(45);
    smoother->SetRelaxationFactor(0.05);
    smoother->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    smoother->Update();

    mesh->DeepCopy( smoother->GetOutput() );

    // Free memory
    smoother->Delete();

    cout << endl << "Done" << endl << endl;
}

/**
 * Export the mesh in STL format.
 * @param mesh Mesh to export.
 * @param path Path to the exported stl file.
 */
void exportAsStlFile( const vtkSmartPointer<vtkPolyData>& mesh, const string& path )
{
    cout << "Mesh export as stl file: " << path << endl;
    vtkSmartPointer<vtkSTLWriter> writer = vtkSTLWriter::New();
    writer->SetFileName( path.c_str() );
    writer->SetInputData( mesh );
    writer->SetFileTypeToASCII();
    writer->Write();
    cout << "Done" << endl << endl;
}

void showUsage()
{
    cout << "How to use dicom2Mesh:" << endl << endl;

    cout << "Minimum example. This creates a mesh file called mesh.stl by using a iso value of 400 (makes bone visible)" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory " << endl << endl;

    cout << "This creates a mesh file called abc.stl by using a iso value of 700" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -o abc.stl  -t 700 " << endl << endl;

    cout << "This creates a mesh with a reduced number of polygons by half" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -r" << endl << endl;

    cout << "This creates a mesh with a reduced number of polygons by 80%" << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -r 0.8" << endl << endl;

    cout << "This creates a mesh where small connected objects are removed. In particular, only connected objects with a minimum number of vertices of 20% of the object with the most vertices are part of the result." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -e  0.2" << endl << endl;

    cout << "This creates a mesh which is shifted to the coordinate system origin." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -c" << endl << endl;

    cout << "This creates a mesh which is smoothed." << endl;
    cout << "> dicom2mesh -i pathToDicomDirectory  -s" << endl << endl;

    cout << "Arguments can be combined." << endl << endl;
}

int main(int argc, char *argv[])
{
    //****** Parse arguments *******//
    Dicom2MeshSettings settings;
    for( unsigned int a = 0; a < argc; a++ )
    {
        string cArg( argv[a] );

        if( cArg.compare("-i") == 0 )
        {
            // next argument is path to dicom directory
            a++;
            if( a < argc )
            {
                settings.pathToDicomDirectory = argv[a];
                settings.pathToDicomSet = true;
            }
            else
            {
                showUsage();
                return -1;
            }
        }
        else if( cArg.compare("-o") == 0 )
        {
            // next argument is file path to mesh output
            a++;
            if( a < argc )
            {
                settings.outputFilePath = argv[a];
                settings.pathToOutputSet = true;
            }
            else
            {
                showUsage();
                return -1;
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
                return -1;
            }
        }
        else if( cArg.compare("-h") == 0 )
        {
            showUsage();
            return 0;
        }
        else if( cArg.compare("-r") == 0 )
        {
            settings.enableMeshReduction = true;
            // next argument is reduction (float)
            a++;
            if( a < argc ) // default value is 0.5
                settings.reductionRate = stof( string(argv[a]) );
        }
        else if( cArg.compare("-e") == 0 )
        {
            settings.extracOnlyBigObjects = true;
            // next argument is size ratio (float)
            a++;
            if( a < argc ) // default value is 0.1
                settings.nbrVerticesRatio = stof( string(argv[a]) );
        }
        else if( cArg.compare("-c") == 0 )
        {
            settings.setOriginToCenterOfMass = true;
        }
        else if( cArg.compare("-s") == 0 )
        {
            settings.enableSmoothing = true;
        }

    }

    if( !settings.pathToDicomSet )
    {
        cerr << "Path to DICOM directory missing" << endl << "> dicom2mesh -i pathToDicom" << endl;
        return -1;
    }

    //******************************//


    vtkSmartPointer<vtkCallbackCommand> progressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    progressCallback->SetCallback(myVtkProgressCallback);


    //******** Read DICOM *********//
    cout << "Read DICOM images located under " << settings.pathToDicomDirectory << endl;
    string progressData("Read DICOM");
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    vtkDICOMImageReader* reader = vtkDICOMImageReader::New();
    reader->SetDirectoryName( settings.pathToDicomDirectory.c_str() );
    reader->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    reader->Update();


    vtkImageData* rawVolumeData = vtkImageData::New();
    rawVolumeData->DeepCopy(reader->GetOutput());

    reader->Delete(); // free memory
    cout << endl << "Done" << endl << endl;
    //******************************//


    //******** Create Mesh ******* //
    cout << "Create surface mesh with iso value = " << settings.isoValue << endl;
    progressData = "Create mesh";
    progressCallback->SetClientData( (void*) (progressData.c_str()) );

    vtkMarchingCubes* surfaceExtractor = vtkMarchingCubes::New();
    surfaceExtractor->ComputeNormalsOn();
    surfaceExtractor->ComputeScalarsOn();
    surfaceExtractor->SetValue( 0, settings.isoValue) ;
    surfaceExtractor->SetInputData( rawVolumeData );
    surfaceExtractor->AddObserver(vtkCommand::ProgressEvent, progressCallback);
    surfaceExtractor->Update();

    vtkSmartPointer<vtkPolyData> mesh = vtkSmartPointer<vtkPolyData>::New();
    mesh->DeepCopy( surfaceExtractor->GetOutput() );

    // free memory
    rawVolumeData->Delete();
    surfaceExtractor->Delete();
    cout << endl << "Done" << endl << endl;
    //******************************//

    if( mesh->GetNumberOfCells() == 0 )
    {
        cerr << "No mesh could be created. Wrong DICOM or wrong iso value" << endl;
        return -1;
    }


    //***** Mesh post-processing *****//
    if( settings.setOriginToCenterOfMass )
        moveMeshToCOSCenter( mesh );

    if( settings.enableMeshReduction )
    {
        // check reduction rate
        if( settings.reductionRate < 0.0 || settings.reductionRate > 1.0 )
            cout << "Reduction skipped due to invalid reductionRate " << settings.reductionRate << " where a value of 0.0 - 1.0 is expected." << endl;
        else
            meshReduction( mesh, settings.reductionRate, progressCallback );
    }

    if( settings.extracOnlyBigObjects )
    {
        if( settings.nbrVerticesRatio < 0.0 || settings.nbrVerticesRatio > 1.0 )
            cout << "Smoothing skipped due to invalid reductionRate " << settings.nbrVerticesRatio << " where a value of 0.0 - 1.0 is expected." << endl;
        else
            removeSmallObjects( mesh, settings.nbrVerticesRatio );
    }

    if( settings.enableSmoothing )
    {
        smoothMesh( mesh, 20, progressCallback );
    }

    //********************************//

    exportAsStlFile( mesh, settings.outputFilePath );

    return 0;
}



