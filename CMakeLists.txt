CMAKE_MINIMUM_REQUIRED(VERSION 3.13.0)
PROJECT(im3d_sample VERSION 0.1.0)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# sub projects
#
SUBDIRS(glew im3d 
    im3d_dx11 samples/sample_dx11 
    im3d_gl3 samples/sample_gl3
    )
