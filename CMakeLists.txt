CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(im3d_sample VERSION 0.1.0)

INCLUDE_DIRECTORIES(
    "."
    )
LINK_DIRECTORIES(
    )
ADD_DEFINITIONS(
    # -DUNICODE
    # -D_UNICDOE
    -DIM3D_OPENGL
    -DIM3D_OPENGL_VSHADER=140
    -DIM3D_OPENGL_VMAJ=3
    -DIM3D_OPENGL_VMIN=0
    -DIM3D_VERTEX_ALIGNMENT=16
    -DGLEW_BUILD
    )
ADD_COMPILE_OPTIONS(
    /std:c++latest 
    /EHsc
    )

FILE(GLOB SRC
        main.cpp
        im3d.cpp
        im3d_example.cpp
        im3d_opengl31.cpp
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        GL/glew.c
    ) 
ADD_EXECUTABLE(${PROJECT_NAME}
    ${SRC}
    )
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    OPENGL32
    )
