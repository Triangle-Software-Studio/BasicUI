cmake_minimum_required(VERSION 3.16)
project(BasicUI VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Dependency paths (override via -D or environment variable)
# ---------------------------------------------------------------------------

# Helper macro: set a cache variable from env var, falling back to a default path
macro(set_dep_path VAR_NAME ENV_NAME DEFAULT_PATH)
    set(_cur_val "${${VAR_NAME}}")
    if(_cur_val STREQUAL "" OR _cur_val MATCHES "NOTFOUND")
        set(_val "$ENV{${ENV_NAME}}")
        if(_val STREQUAL "")
            set(_val "${DEFAULT_PATH}")
        endif()
        if(NOT _val STREQUAL "")
            set(${VAR_NAME} "${_val}" CACHE PATH "Path to ${VAR_NAME}" FORCE)
        endif()
    endif()
endmacro()

set_dep_path(SDL2_DIR SDL2_DIR "D:/libs/SDL2/cmake")
set_dep_path(GLEW_DIR GLEW_DIR "D:/libs/glew")
set_dep_path(FREETYPE_DIR FREETYPE_DIR "D:/libs/freetype")

# ---------------------------------------------------------------------------
# Find SDL2
# ---------------------------------------------------------------------------
if(SDL2_DIR AND EXISTS "${SDL2_DIR}/SDL2Config.cmake")
    find_package(SDL2 REQUIRED PATHS ${SDL2_DIR} NO_DEFAULT_PATH)
else()
    # Fallback: search in standard locations / PATH
    find_package(SDL2 REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Find OpenGL
# ---------------------------------------------------------------------------
find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# Find / configure GLEW
# ---------------------------------------------------------------------------
if(GLEW_DIR)
    set(GLEW_INCLUDE_DIR "${GLEW_DIR}/include" CACHE PATH "GLEW include dir" FORCE)

    if(MINGW OR CMAKE_COMPILER_IS_GNUCC)
        # MinGW prefers .a or .dll.a
        find_library(GLEW_LIBRARY
            NAMES glew32 glew32s libglew32 libglew32dll
            PATHS "${GLEW_DIR}/lib/Release/x64" "${GLEW_DIR}/lib" "${GLEW_DIR}/lib/Release/Win32"
            NO_DEFAULT_PATH
        )
    else()
        # MSVC
        find_library(GLEW_LIBRARY
            NAMES glew32 glew32s
            PATHS "${GLEW_DIR}/lib/Release/x64" "${GLEW_DIR}/lib"
            NO_DEFAULT_PATH
        )
    endif()

    if(NOT GLEW_LIBRARY)
        message(FATAL_ERROR "GLEW library not found in ${GLEW_DIR}/lib. Set GLEW_DIR correctly.")
    endif()
else()
    message(FATAL_ERROR "GLEW_DIR not set. Please set GLEW_DIR to the GLEW root directory.")
endif()

# ---------------------------------------------------------------------------
# Find / configure FreeType
# ---------------------------------------------------------------------------
if(FREETYPE_DIR)
    find_path(FREETYPE_INCLUDE_DIR ft2build.h
        PATHS "${FREETYPE_DIR}/include"
        NO_DEFAULT_PATH
    )
    if(NOT FREETYPE_INCLUDE_DIR)
        message(FATAL_ERROR "FreeType include dir not found. Set FREETYPE_DIR correctly.")
    endif()

    find_library(FREETYPE_LIBRARY
        NAMES freetype libfreetype
        PATHS "${FREETYPE_DIR}/build/Release" "${FREETYPE_DIR}/lib" "${FREETYPE_DIR}/objs/.libs"
        NO_DEFAULT_PATH
    )
    if(NOT FREETYPE_LIBRARY)
        message(FATAL_ERROR "FreeType library not found. Set FREETYPE_DIR correctly.")
    endif()
else()
    message(FATAL_ERROR "FREETYPE_DIR not set. Please set FREETYPE_DIR to the FreeType root directory.")
endif()

add_library(Freetype::Freetype INTERFACE IMPORTED)
set_target_properties(Freetype::Freetype PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${FREETYPE_LIBRARY}"
)

# On some platforms SDL2 is imported as a target, on others variables.
if(NOT TARGET SDL2::SDL2)
    message(FATAL_ERROR "SDL2::SDL2 target not found. Adjust SDL2_DIR or install SDL2 development libraries.")
endif()

# ---------------------------------------------------------------------------
# BasicUI library
# ---------------------------------------------------------------------------

add_library(basicui STATIC
    src/platform/window.cpp
    src/render/text_grid.cpp
    src/render/glyph_atlas.cpp
    src/render/renderer.cpp
    src/render/shader.cpp
    src/style/font.cpp
    src/layout/layout.cpp
    src/layout/flex_layout.cpp
    src/layout/grid_layout.cpp
    src/widget/widget.cpp
    src/widget/label.cpp
    src/widget/button.cpp
    src/widget/input.cpp
    src/widget/checkbox.cpp
    src/widget/scrollbar.cpp
    src/widget/panel.cpp
    src/widget/screen.cpp
    src/widget/menubar.cpp
    src/widget/listview.cpp
    src/widget/progressbar.cpp
    src/widget/radiobutton.cpp
    src/widget/combobox.cpp
    src/widget/tab.cpp
    src/widget/tooltip.cpp
)

target_include_directories(basicui
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${GLEW_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

target_link_libraries(basicui
    PUBLIC
        SDL2::SDL2
        Freetype::Freetype
        OpenGL::GL
        ${GLEW_LIBRARY}
)

# On Windows you may need to link SDL2main and dwmapi explicitly.
if(WIN32)
    target_link_libraries(basicui PUBLIC SDL2::SDL2main dwmapi)
endif()

# ---------------------------------------------------------------------------
# Demo executable
# ---------------------------------------------------------------------------

add_executable(basicui_demo demo/main.cpp)

target_link_libraries(basicui_demo PRIVATE basicui)

# ---------------------------------------------------------------------------
# Install rules (optional)
# ---------------------------------------------------------------------------

install(TARGETS basicui basicui_demo
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include)
