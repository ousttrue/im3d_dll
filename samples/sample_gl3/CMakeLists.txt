SET(SUBNAME sample_gl3)
ADD_EXECUTABLE(${SUBNAME}
    )
TARGET_SOURCES(${SUBNAME} PRIVATE
    main.cpp
    ../orbit_camera.cpp
    ../win32_window.cpp
    )
TARGET_COMPILE_OPTIONS(${SUBNAME} PRIVATE
    /std:c++latest 
    /EHsc
    )
TARGET_INCLUDE_DIRECTORIES(${SUBNAME} PRIVATE
    .
    ../../im3d
    ../../im3d_gl3
    )
TARGET_COMPILE_DEFINITIONS(${SUBNAME} PRIVATE
    NOMINMAX
    UNICODE
    _UNICDOE
    IM3D_VERTEX_ALIGNMENT=16
    )
TARGET_LINK_LIBRARIES(${SUBNAME}
    WINMM
    im3d
    im3d_gl3
    )
