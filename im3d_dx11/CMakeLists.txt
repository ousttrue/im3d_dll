SET(SUBNAME im3d_dx11)
ADD_LIBRARY(${SUBNAME} SHARED)
CMAKE_POLICY(SET CMP0076 NEW) # CMakeが自動的に相対パスを絶対パスへ変換する
TARGET_SOURCES(${SUBNAME} PRIVATE
    dx11_renderer.cpp
    im3d_impl_dx11.cpp
    )
TARGET_COMPILE_OPTIONS(${SUBNAME} PRIVATE
    /std:c++latest 
    /EHsc
    )
TARGET_INCLUDE_DIRECTORIES(${SUBNAME} PRIVATE
    .
    ../im3d
    )
TARGET_COMPILE_DEFINITIONS(${SUBNAME} PRIVATE
    EXPORT_IM3D_DX11
    )
TARGET_LINK_LIBRARIES(${SUBNAME} PRIVATE
    im3d
    d3d11
    d3dcompiler
    )
