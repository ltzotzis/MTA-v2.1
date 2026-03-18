# MTA-v2.1
A refresh of the Manchester Thermal Analyzer to keep up with dependencies and new support for GPU powered solvers.

The two ways to use the MTA are described in the following sections.
## Apptainer container
Allows for easy use, only required dependency being Apptainer. Usage is via the Perl script.

## Compiling from source
The best option if there is a need to customize the MTA or use system libraries for better performance. While requiring compilation of the heat executable via the setup script provided in the mta_share subfolder, the process should be automatic. System dependencies for the setup script are gcc/g++, CMake and MPI. Other dependencies (Hypre, PETSc, Deal.ii) are downloaded by the setup script automatically. Modification of the setup script in order to either use one's own versions of these libraries or to configure them in another way is possible. If compiling from source, there are two further options:
+ Pure CPU implementation. Skips the need for CUDA installation.
+ Implementation that can utilize GPU powered solvers. Can be useful for fine meshes/transient simulation with long power traces. Requires CUDA as an additional dependency.
