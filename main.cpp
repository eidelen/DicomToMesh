#include <iostream>
#include <string>

#include <vtkCallbackCommand.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>


// Note: To safe memory, smart-pointers were not used for certain objects.
//       This has the advantage that huge memory blocks can be released
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



