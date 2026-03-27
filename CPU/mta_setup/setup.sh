root_dir=$(pwd)
echo "Working in ${root_dir}"

git clone https://github.com/petsc/petsc.git
cd petsc
git checkout 49c2f9045d3a91e7be182f7f53e05380f8af9fad
./configure --prefix="${root_dir}/petsc_install" --download-fblaslapack=1 --download-hypre --with-debugging=no -COPTFLAGS="-O3" -CXXOPTFLAGS="-O3" -FOPTFLAGS="-O3" 
make -j4 PETSC_DIR="${root_dir}/petsc" PETSC_ARCH=arch-linux-c-opt all
make -j4 PETSC_DIR="${root_dir}/petsc" PETSC_ARCH=arch-linux-c-opt install

cd ..
git clone https://github.com/dealii/dealii.git
cd dealii
git checkout a376925da1ef7ada9ec4708af9c15b7b5ef9e5b0
mkdir build
cd build
echo "-DDEAL_II_WITH_PETSC=ON -DDEAL_II_WITH_MPI=ON -DMPI_CXX_COMPILER=${root_dir}/petsc_install/bin/mpicxx -DMPI_C_COMPILER=${root_dir}/petsc_install/bin/mpicc -DCMAKE_INSTALL_PREFIX=${root_dir}/dealii_install/ -DPETSC_DIR=${root_dir}/petsc_install/ -DMPI_Fortran_COMPILER=${root_dir}/petsc_install/bin/mpif90"
cmake .. -DDEAL_II_WITH_PETSC=ON -DDEAL_II_WITH_MPI=ON -DCMAKE_INSTALL_PREFIX="${root_dir}/dealii_install/" -DPETSC_DIR="${root_dir}/petsc_install/"
make -j4 install

cd ../../mta
cd mta
sed -i "42,43s|/|${root_dir}/|" CMakeLists.txt
g++ -static -std=c++11 -O3 -funroll-loops -c tinyxml2.cpp -o lib/tinyxml2.o
g++ -static -std=c++11 -O3 -funroll-loops -c mta_util.cpp -o lib/mta_util.o
cd build
cmake -DDEAL_II_DIR="${root_dir}/dealii_install/" ..
make release
make

