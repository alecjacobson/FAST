# FAST
--------
Copyright Alec Jacobson 2015

Cocoa and GLUT applications wrapping the Skinning class for prototyping linear
blend skinning and other real time deformations methods. This also functions as
the demo code for the SIGGRAPH paper "Fast Automatic Skinning Transformations"
by Jacobson et al. 2013. To jump to the relevant code for that paper, see
`igl/arap_dof.h` in [libigl](https://github.com/libigl/libigl/)

### Note about name ###
The previous name of the project files was "skinning". Since this program is
really a prototype program for "Fast Automatic Skinning Transformations", this
old name was a bit too general. Nevertheless some of the files and projects
keep this name as a legacy to avoid the ugly refactoring required when
renaming.

### Dependencies ###
- Eigen3 (matrix library)
- libigl (IGL's header library)
-   AntTweakBar (OpenGL GUI toolkit)
- OpenGL (realtime rendering)
- Cocoa (UI on Mac OS X)
- GLUT (extra openGL utilities: displaying text, UI on everything else)
- Optional dependencies
  - tetgen (tetrahedral meshing)
  - mosek (quadratic programming optimization)

### Compilation notes ###
Eigen's Block.h was giving some warnings, so I've temporarily turned of
warnings for implicitly converting 64-bit types to 32-bit types:
`GCC_WARN_64_TO_32_BIT_CONVERSION`
  

## Installation ##

### [libigl](https://github.com/libigl/libigl/) ### 
    
#### Eigen3 ####
Eigen3 is a header library for matrix math. Install on mac os x using

    sudo port install eigen3

#### AntTweakBar ####
AntTweakBar is an OpenGL/DirectX library for simple UI. Use the version in
`$LIBIGL/external/AntTweakBar`

    cd $LIBIGL/external/AntTweakBar/src
    make -f Makefile.static

Compile libigl with:
    
    cd $LIBIGL
    make

You may have to edit your profile in `$LIBIGL/Makefile.conf` appropriately.

### Compilation and Execution ###

#### Linux/Unix command line ####

    cd skinning/
    make
    ./skinning ogre/

#### Mac OS X ####

    open skinning.xcodeproj
    Build and Run

#### Windows ####
Completely possible. Not supported.


## Usage ##
To run the program issue

    ./skinning ogre/

This will open the ogre example. 

Press 't' to toggle displaying the filled triangles of the mesh.

Press 'l' to toggle displaying the wireframe of the mesh.

The "state" of this program is partly determined by which shader is being
used. To toggle between shaders, use the '<' and '>' keys. 

Switch to the LBS shader.

Press 's' to toggle displaying the skeleton

Click on individual bones or endpoints to select them.

Press 'r' to reset all bones to their rest positions.

Press 'R' to reset selected bones.

Right-click and drag on an endpoint to rotate the bone about its parent

Right-click and drag on a bone to wist about its axis

Left-click and drag on a bone or endpoint to translate (not so useful, yet)

The "Fast Automatic Skinning Transformations" method may be activated by
setting appropriate parameters in the "AutoDOF" group of the GUI and
toggling "Auto DOF" by pressing 'D'.

Press 'D' to turn on "Auto DOF" and wait for precomputation to complete.

Left-click and drag on Yellow endpoints to set position constraints.

Select an endpoint and hit 'f' to toggle its type:

|Color  | Constraint type                                |
|-------|------------------------------------------------|
|Red    | fully constrained                              |
|Pink   | linearly constrained (position is free)        |
|Yellow | positionally constrained (linear part is free) |
|Green  | completely free                                |

Right-click and drag red endpoints to apply full rigid transformations

Help for the GUI is available by clicking the little question mark in the
bottom left corner.

## Zipping ##
Zip this direcotry without .hg litter using:
  
    make -C skinning clean
    rm -rf build
    zip -9 -r --exclude=@exclude.lst skinning.zip ../skinning

## Bundling dependencies
Download Alec's [patched version of
dylibbundler](https://github.com/alecjacobson/macdylibbundler) and run the
following:

    dylibbundler -od -b -x ./FAST.app/Contents/MacOS/FAST -d ./FAST.app/Contents/libs/
    install_name_tool -change @loader_path/libmosek64.7.0.dylib @executable_path/../libs/libmosek64.7.0.dylib  ./FAST.app/Contents/MacOS/FAST

## Contact ##
Please contact [Alec Jacobson](mailto:alecjacobson@gmail.com) if you have
questions or comments.
