cmake_minimum_required(VERSION 3.16)
project(FFModMenu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find DirectX SDK
set(DXSDK_DIR "$ENV{DXSDK_DIR}" CACHE PATH "DirectX SDK Directory")

# Source files
set(SOURCES
    src/main.cpp
    src/memory.cpp
    src/overlay.cpp
    src/features.cpp
    src/menu.cpp
    src/config.cpp
    # ImGui
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_impl_dx9.cpp
    imgui/imgui_impl_win32.cpp
)

set(HEADERS
    src/memory.h
    src/overlay.h
    src/features.h
    src/menu.h
    src/config.h
    src/offsets.h
    imgui/imgui.h
    imgui/imgui_impl_dx9.h
    imgui/imgui_impl_win32.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/imgui
    ${DXSDK_DIR}/Include
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${DXSDK_DIR}/Lib/x86
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d9
    d3dx9
    dwmapi
)

# Compiler flags
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()
