### CMakeLists.txt for CUDA

cmake_minimum_required(VERSION 2.8)
enable_language(CUDA)


# Specify include directories
include_directories(
  src
)

# Specify library paths
link_directories(
)

# For compilation ...
# Specify target & source files to compile it from
add_executable(
  hellocuda
  src/sdl.cu
  src/triangulation/s_hull_pro.cpp
  src/triangulation/DelaunayTriang.cpp
)

# For linking ...
# Specify target & libraries to link it with
target_link_libraries(
  hellocuda
  SDL2
  SDL2_gfx
)

target_compile_features(hellocuda PUBLIC cxx_std_11)

set_property(TARGET hellocuda PROPERTY CUDA_STANDARD 11)
string(APPEND CMAKE_CUDA_FLAGS " -gencode arch=compute_61,code=sm_61 ")
