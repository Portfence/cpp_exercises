include(ExternalProject)
include(ProcessorCount)
find_package(Threads REQUIRED)
find_program(FFMPEG_MAKE_EXECUTABLE NAMES gmake make REQUIRED)
find_program(FFMPEG_NASM_EXECUTABLE NAMES nasm)

# Pin both the release and archive contents for repeatable downloads.
set(FFMPEG_VERSION 8.1.3)
set(FFMPEG_URL_HASH "SHA256=7138d28c96d9d3e3af4ee3d8cad72741f8ffb40da90c1112235dea3ecd3178a3")
set(FFMPEG_PREFIX "${CMAKE_BINARY_DIR}/_deps/ffmpeg-${FFMPEG_VERSION}")
set(FFMPEG_INSTALL_DIR "${FFMPEG_PREFIX}/install")

ProcessorCount(FFMPEG_JOBS)
if(FFMPEG_JOBS LESS 1)
    set(FFMPEG_JOBS 1)
elseif(FFMPEG_JOBS GREATER 8)
    set(FFMPEG_JOBS 8)
endif()

set(FFMPEG_ASM_OPTIONS)
if(FFMPEG_NASM_EXECUTABLE)
    list(APPEND FFMPEG_ASM_OPTIONS "--x86asmexe=${FFMPEG_NASM_EXECUTABLE}")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i.86)$")
    message(WARNING "NASM not found: FFmpeg CPU decoding will lack x86 assembly optimizations. Install nasm and reconfigure to enable them.")
    list(APPEND FFMPEG_ASM_OPTIONS --disable-x86asm)
endif()

find_package(PkgConfig REQUIRED)

set(NVCODEC_INSTALL_DIR
    "${CMAKE_BINARY_DIR}/_deps/nvcodec-12.1.14.0/install")

ExternalProject_Add(nvcodec_headers
    GIT_REPOSITORY https://github.com/FFmpeg/nv-codec-headers.git
    GIT_TAG n12.1.14.0
    GIT_SHALLOW TRUE
    UPDATE_COMMAND ""

    PREFIX "${CMAKE_BINARY_DIR}/_deps/nvcodec-12.1.14.0"
    INSTALL_DIR "${NVCODEC_INSTALL_DIR}"

    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND
        "${FFMPEG_MAKE_EXECUTABLE}" -C <SOURCE_DIR>
        "PREFIX=<INSTALL_DIR>" LIBDIR=lib install
)

# FFmpeg uses configure/make, independently of this project's CMake generator.
# Build demuxing/decoding libraries only. GPU backends can be enabled here later.
ExternalProject_Add(ffmpeg_external
    URL "https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.xz"
    URL_HASH "${FFMPEG_URL_HASH}"
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    PREFIX "${FFMPEG_PREFIX}"
    INSTALL_DIR "${FFMPEG_INSTALL_DIR}"
    CONFIGURE_COMMAND
        "${CMAKE_COMMAND}" -E env
        "PKG_CONFIG_PATH=${NVCODEC_INSTALL_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}"
        <SOURCE_DIR>/configure
        --prefix=<INSTALL_DIR>
        --libdir=<INSTALL_DIR>/lib
        --cc=${CMAKE_C_COMPILER}
        --enable-static
        --disable-shared
        --enable-pic
        --disable-programs
        --disable-doc
        --disable-autodetect
        --disable-network
        --disable-avdevice
        --disable-avfilter
        --disable-swscale
        --disable-swresample
        --disable-encoders
        --disable-muxers
        --enable-ffnvcodec
        --enable-cuda
        --enable-nvdec
        --enable-hwaccel=hevc_nvdec
        ${FFMPEG_ASM_OPTIONS}
    BUILD_COMMAND "${FFMPEG_MAKE_EXECUTABLE}" -j${FFMPEG_JOBS}
    INSTALL_COMMAND "${FFMPEG_MAKE_EXECUTABLE}" install
    INSTALL_BYPRODUCTS
        "${FFMPEG_INSTALL_DIR}/lib/libavformat.a"
        "${FFMPEG_INSTALL_DIR}/lib/libavcodec.a"
        "${FFMPEG_INSTALL_DIR}/lib/libavutil.a"
    LOG_CONFIGURE TRUE
    LOG_BUILD TRUE
    LOG_INSTALL TRUE
    LOG_OUTPUT_ON_FAILURE TRUE
    DEPENDS nvcodec_headers
)

# Imported include paths must exist when CMake generates the build system.
file(MAKE_DIRECTORY "${FFMPEG_INSTALL_DIR}/include")
foreach(component IN ITEMS avutil avcodec avformat)
    add_library(FFmpeg::${component} STATIC IMPORTED GLOBAL)
    set_target_properties(FFmpeg::${component} PROPERTIES
        IMPORTED_LOCATION "${FFMPEG_INSTALL_DIR}/lib/lib${component}.a"
        INTERFACE_INCLUDE_DIRECTORIES "${FFMPEG_INSTALL_DIR}/include"
    )
    add_dependencies(FFmpeg::${component} ffmpeg_external)
endforeach()
set_target_properties(FFmpeg::avutil PROPERTIES
    INTERFACE_LINK_LIBRARIES "Threads::Threads;${CMAKE_DL_LIBS};m")
set_target_properties(FFmpeg::avcodec PROPERTIES
    INTERFACE_LINK_LIBRARIES "FFmpeg::avutil")
set_target_properties(FFmpeg::avformat PROPERTIES
    INTERFACE_LINK_LIBRARIES "FFmpeg::avcodec;FFmpeg::avutil")
