# DicomToMesh

DicomToMesh is a handy command line tool, which enables the user to automatically create a 3D mesh from a set of 2D DICOM images, a common image format used in medicine. The supported 3D mesh formats are STL, OBJ and PLY. DicomToMesh works on Linux, OSX and Windows.

<p align="center"><img alt="dicom2mesh" src="http://eidelen.diffuse.ch/dicomtomesh.png" width="80%"></p>

# Mesh Creation

The 3D surface mesh is computed by the marching cubes algorithm. As an input, this algorithm requires a threshold which indicates what range of voxel values should be considered. This threshold is also known as iso-value. In CT scans, the iso-value depends on the tissue (density). In Dicom2Mesh you can specify the iso-value with the parameter <code>-t X</code>, where X is an integer.

# Mesh Post-Processing Options

**Mesh reduction:** The mesh of a medical DICOM image can easily exceed 1 GB of data. Reducing the number of polygons, therefore, is a crucial feature. Reduction can be enabled by the option <code>-r X</code> where X is a floating-point value between 0.0 and 1.0. 

<p align="center"><img alt="reduction" src="http://eidelen.diffuse.ch/mesh-reduction.png" width="60%"></p>

**Mesh smoothing:** Acquired medical images contain often heavy noise. This is visible in the extracted 3D surface. Smoothing the mesh leads often to a better result. Smoothing can be enabled with the argument <code>-s</code>.

<p align="center"><img alt="smoothing" src="http://eidelen.diffuse.ch/mesh-smoothed.png" width="60%"></p>

**Remove small objects:** The resulting 3D mesh contains often parts which are not of interest, such as for example the screws of the table on which a CT scan of a patient was acquired. With DicomToMesh, you can remove objects below a certain size by adding the option <code>-e X</code> where X is a floating-point value between 0.0 and 1.0. <code>X</code> is a size threshold relative to the connected object with the most vertices. It is easy understandable with an example: The biggest connected object of the mesh has 1000 vertices. Then <code>-e 0.25</code> removes all connected objects with less than 250 vertices.    

<p align="center"><img alt="filter" src="http://eidelen.diffuse.ch/mesh-filter.png" width="80%"></p>


# Visualisation 

By passing the command line option <code>-v</code> the resulting mesh is visualised in a 3D environment. By double clicking the mesh's  surface, the corresponding 3D coordinate is printed to the shell.

<p align="center"><img alt="filter" src="http://eidelen.diffuse.ch/mesh-visualization.png" width="50%"></p>


# Installation

Ubuntu 18.04
```
> sudo add-apt-repository ppa:eidelen/d2m
> sudo apt-get update
> sudo apt-get install dicom2mesh
```

On OSX and Windows it is required to build DicomToMesh yourself. 

# Building

The software is written in C++11 and uses VTK 7.0. CMake is used as build-system.

Building DicomToMesh on Linux and OSX:
```
> git clone https://github.com/AOT-AG/DicomToMesh.git
> cd DicomToMesh
> mkdir build
> cd build
> ccmake ..  #opens ccmake window
```
Within the ccmake window, pass the path to your vtk installation or vtk build directory. In my case, it looks like that
```
BUILD_GUI                        OFF
CMAKE_BUILD_TYPE                 DEBUG
CMAKE_INSTALL_PREFIX             /usr/local                                   
USE_VTK_DICOM                    OFF                                          
VTK_DIR                          /home/eidelen/Development/libs/vtk/build     
```
Press <code>c</code> and then <code>g</code>. 
Now you are back in terminal and ready to build.
```
> make
```
In order to extend the supported DICOM formats, the libray vtk-dicom <code>USE_VTK_DICOM</code> can be optionally enabled (see https://github.com/dgobbi/vtk-dicom).

Building on Windows:

Building on Windows is essentially the same as building under Linux and OSX. The project settings can be done with the cmake GUI. The versions we are using are VTK 7.0.0 together with Visual Studio 2015. The optional GUI we are building with QT 5.9.4. 

# How to use Dicom2Mesh

Command line arguments can be combined and passed in arbitrary order.

**Input and output:** The path to the DICOM directory is passed by the argument <code>-i dicomPath</code>. The file name of the resulting 3D mesh is specified by the parameter <code>-o meshPath</code>. This simple example transforms a DICOM data set into a 3D mesh file called mesh.stl, by using an iso-value of 557 <code>-t 557</code>. 

<code>> dicom2mesh -i pathToDicomDirectory -t 557 -o mesh.stl</code>

Alternatively, one can use an existing 3d mesh as input. This is useful if you want to apply only mesh post-processing routines and bypass the time consuming mesh creation step. This example imports the former mesh.stl, centers it and exports it in the OBJ mesh format.

<code>> dicom2mesh -i mesh.stl -c -o newMesh.obj</code>

**Mesh post-processing:** The following example shows different mesh post-processing methods applied to resulting surface mesh out of the marching cubes algorithm. In particular, a mesh is reduced by 90% of its original number faces <code>-r 0.9</code>. In addition, the mesh is smoothed <code>-s</code> and centred at the coordinate system's origin <code>-c</code>.  Another helpful function is the removal of small objects - here the removal of every object smaller than 5% of the biggest object <code>-e 0.05</code>.

<code>> dicom2mesh -i pathToDicomDirectory -r 0.9 -s -c -e 0.05 -o mesh.stl</code>

# GUI

Dicom2Mesh can be built with a small GUI on top.
This showed up to be helpful for users without command line experience.
However, the control possibilities are limited compared to the command line version.

<p align="center"><img alt="dicom2mesh_gui" src="http://eidelen.diffuse.ch/dicomtomesh_gui2.png" width="75%"></p>

In order to build the Dicom2Mesh GUI, please set the cmake flag <code>BUILD_GUI=ON</code> when configuring the project.
The GUI requires QT.

The icons in the gui were made by <a href="https://www.flaticon.com/authors/smashicons" title="Smashicons">Smashicons</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a>. The license is <a href="http://creativecommons.org/licenses/by/3.0/" title="Creative Commons BY 3.0" target="_blank">CC 3.0 BY</a>

# Contributors

DicomToMesh is a small in-house product of AOT AG (http://www.aot.swiss). Since it is based on several open-source examples, we decided to make our code public as well. We hope somebody can use parts of it. Participants are most welcome.

Have fun :)

<p align="right"><img alt="filter" src="http://eidelen.diffuse.ch/aot.jpg" width="30%"></p>
