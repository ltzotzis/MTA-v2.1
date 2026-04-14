# MTA-v2.1
An updated version of the Manchester Thermal Analyzer to keep up with dependencies and new support for GPU powered solvers. It can be used via the customizable perl script or via invoking the executables directly.

 Maintenance has been performed and GPU acceleration support for the solvers has been added. 

The two ways to use the MTA are described in the following sections.

## Apptainer container
Allows for easy use, only required dependency being Apptainer. Great as a fallback option if having trouble compiling from source. One can either use the perl script normally, or to invoke the individual executables: 
```
apptainer exec mta2.sif executablename
```

## Compiling from source
The best option if there is a need to customize the MTA or use system libraries for better performance. While requiring compilation of the heat executable via the setup script provided in the mta_setup subfolder, the process should be automatic via the provided setup script. System dependencies for the setup script are gcc/g++, CMake and MPI. Other dependencies (Hypre, PETSc, Deal.ii) are downloaded by the setup script automatically. Modification of the setup script in order to either use one's own versions of these libraries or to configure them in another way is possible. The installation via this method is:

+ Compile dependencies and heat executable via the setup script
+ Move/copy the heat executable that appears in mta_setup/mta/build to the bin folder

When compiling from source, there are two further options:
+ Pure CPU implementation. Skips the need for CUDA installation.
+ Implementation that can utilize GPU powered solvers. Can be useful for fine meshes/transient simulation with long power traces. Requires CUDA as an additional dependency.

The other executables are fully static and provided as binaries in all versions. If desired, they can also be compiled via make or via the commands:
+ msh_gen:
  - `g++ -static -std=c++11 -O3 -funroll-loops -c tinyxml2.cpp -o lib/tinyxml2.o`
  - `g++ -static -std=c++11 -O3 -funroll-loops -o msh_gen msh_gen.cpp lib/tinyxml2.o`
+ pkxml:
  - `g++ -static -std=c++11 -O3 -funroll-loops -c tinyxml2.cpp -o lib/tinyxml2.o`
  - `g++ -static -std=c++11 -O3 -funroll-loops -o pkxml pkxml.cpp lib/tinyxml2.o`
+ diexml:
  - `g++ -static -std=c++11 -O3 -funroll-loops -c tinyxml2.cpp -o lib/tinyxml2.o`
  - `g++ -static -std=c++11 -O3 -funroll-loops -I./lefdef/def/include -I./lefdef/lef/include -L./lefdef/def/lib -L./lefdef/lef/lib -o diexml diexml.cpp lib/*.o -ldef -llef`
    
    To compile diexml the lefdef library (included as a submodule) must be compiled.

To use the GPU powered solvers, simply add the -GPU option when running the heat simulator executable.




