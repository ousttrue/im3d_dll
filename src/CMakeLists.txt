SET(SUBNAME im3d_sample)
FILE(GLOB SRC
        *.cpp
    ) 
ADD_EXECUTABLE(${SUBNAME}
    ${SRC}
    )
TARGET_COMPILE_OPTIONS(${SUBNAME} PRIVATE
    /std:c++latest 
    /EHsc
    )
TARGET_INCLUDE_DIRECTORIES(${SUBNAME} PRIVATE
    .
    ../im3d
    ../im3d_gl3
    )
TARGET_COMPILE_DEFINITIONS(${SUBNAME} PRIVATE
    NOMINMAX
    UNICODE
    _UNICDOE
    IM3D_VERTEX_ALIGNMENT=16
    )
TARGET_LINK_LIBRARIES(${SUBNAME}
    OPENGL32
    WINMM
    glew
    im3d
    im3d_gl3
    )
