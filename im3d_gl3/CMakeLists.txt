SET(SUBNAME im3d_gl3)
ADD_LIBRARY(${SUBNAME} SHARED)
CMAKE_POLICY(SET CMP0076 NEW) # CMakeが自動的に相対パスを絶対パスへ変換する
TARGET_SOURCES(${SUBNAME} PRIVATE
    wgl_context.cpp
    gl3_renderer.cpp
    im3d_impl_gl3.cpp
    )
TARGET_INCLUDE_DIRECTORIES(${SUBNAME} PRIVATE
    .
    ../glew
    ../im3d
    )

# ADD_DEFINITIONS(
#     -DIM3D_OPENGL_VSHADER=140
#     -DIM3D_OPENGL_VMAJ=3
#     -DIM3D_OPENGL_VMIN=0
#     )

TARGET_COMPILE_DEFINITIONS(${SUBNAME} PRIVATE
    EXPORT_IM3D_GL3
    )
TARGET_LINK_LIBRARIES(${SUBNAME}
    glew
    im3d
    )
