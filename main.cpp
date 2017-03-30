#include <iostream>
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
    string outputFilePath;
    bool pathToOutputSet = false;
    int isoValue = 400; // Hard Tissue
};


void vtkProgressCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
    vtkAlgorithm* filter = static_cast<vtkAlgorithm*>(caller);
    char* task = static_cast<char*>(clientData);
    cout << '\r';
    cout << task << ": " << filter->GetProgress() * 100 << "%";
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
}

/**
 * Reduces the size / details of a 3D mesh.
 * @param mesh The input mesh. Mesh will be modified afterwards.
 * @param reduction Reduction factor. 0.1 is little reduction. 0.9 is strong reduction.
 * @param progressCallback Progress callback function pointer.
 */
void meshReduction( vtkSmartPointer<vtkPolyData> mesh, const float& reduction, vtkSmartPointer<vtkCallbackCommand> progressCallback )
{
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
}

/**
 * Export the mesh in STL format.
 * @param mesh Mesh to export.
 * @param path Path to the exported stl file.
 */
void exportAsStlFile( const vtkSmartPointer<vtkPolyData>& mesh, const string& path )
{
    vtkSmartPointer<vtkSTLWriter> writer = vtkSTLWriter::New();
    writer->SetFileName( path.c_str() );
    writer->SetInputData( mesh );
    writer->SetFileTypeToASCII();
    writer->Write();
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
        }
    }

    if( !settings.pathToDicomSet )
    {
        cerr << "Path to DICOM directory missing" << endl << "> dicom2mesh -i pathToDicom" << endl;
        return -1;
    }

    if( !settings.pathToOutputSet )
        settings.outputFilePath = "mesh.stl";
    //******************************//


    vtkSmartPointer<vtkCallbackCommand> progressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    progressCallback->SetCallback(vtkProgressCallback);


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


    //********************************//








    return 0;
}



