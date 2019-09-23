SET(SUBNAME sample_dx11)
ADD_EXECUTABLE(${SUBNAME}
    )
TARGET_SOURCES(${SUBNAME} PRIVATE
    dx11_context.cpp
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
    ../../im3d_dx11
    )
TARGET_COMPILE_DEFINITIONS(${SUBNAME} PRIVATE
    NOMINMAX
    UNICODE
    _UNICDOE
    )
TARGET_LINK_LIBRARIES(${SUBNAME}
    WINMM
    im3d
    im3d_dx11
    d3d11
    )
