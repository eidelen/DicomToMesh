# DicomToMesh

This is a handy tool to create a 3D mesh from a set of 2D DICOM images. The segmentation is performed by thresholding and the surface is generated with the marching cubes algorithm. The tool supports mesh reduction, mesh smoothing and removing of small objects. The 3D mesh is exported in STL format.
The software is written in C++ and uses VTK 7.0. CMake is used as build-system.

The code was written for the medical planning and navigation library of AOT AG (http://www.aot.swiss). Since it is based on several open-source examples, I decided to make our code public as well. Hope somebody can use parts of the code. 

Have fun,
Adrian