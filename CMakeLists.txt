cmake_minimum_required (VERSION 3.0)

#-------------------------------------------------------------------------------

project (samplerate)

file(GLOB SAMPLERATE_SOURCES "dependencies/sources/libsamplerate-0.1.9/src/*.c")

include_directories(
	src/common
	dependencies/sources/libsamplerate-0.1.9/src
	dependencies/sources/libsamplerate-0.1.9/Linux_x86
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

add_library(samplerate STATIC ${SAMPLERATE_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(samplerate PRIVATE "-w")
endif()

#-------------------------------------------------------------------------------

project (ogg)

file(GLOB OGG_SOURCES "dependencies/sources/libogg-1.3.3/src/*.c")

include_directories(
	dependencies/sources/libogg-1.3.3/include
	dependencies/sources/libogg-1.3.3/linux_x86/include
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

add_library(ogg STATIC ${OGG_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(ogg PRIVATE "-w")
endif()

#-------------------------------------------------------------------------------

project (vorbis)

file(GLOB VORBIS_SOURCES
	"dependencies/sources/libvorbis-1.3.6/lib/analysis.c"
	"dependencies/sources/libvorbis-1.3.6/lib/bitrate.c"
	"dependencies/sources/libvorbis-1.3.6/lib/block.c"
	"dependencies/sources/libvorbis-1.3.6/lib/codebook.c"
	"dependencies/sources/libvorbis-1.3.6/lib/envelope.c"
	"dependencies/sources/libvorbis-1.3.6/lib/floor0.c"
	"dependencies/sources/libvorbis-1.3.6/lib/floor1.c"
	"dependencies/sources/libvorbis-1.3.6/lib/info.c"
	"dependencies/sources/libvorbis-1.3.6/lib/lookup.c"
	"dependencies/sources/libvorbis-1.3.6/lib/lpc.c"
	"dependencies/sources/libvorbis-1.3.6/lib/lsp.c"
	"dependencies/sources/libvorbis-1.3.6/lib/mapping0.c"
	"dependencies/sources/libvorbis-1.3.6/lib/mdct.c"
	"dependencies/sources/libvorbis-1.3.6/lib/psy.c"
	"dependencies/sources/libvorbis-1.3.6/lib/registry.c"
	"dependencies/sources/libvorbis-1.3.6/lib/res0.c"
	"dependencies/sources/libvorbis-1.3.6/lib/sharedbook.c"
	"dependencies/sources/libvorbis-1.3.6/lib/smallft.c"
	"dependencies/sources/libvorbis-1.3.6/lib/synthesis.c"
	"dependencies/sources/libvorbis-1.3.6/lib/vorbisenc.c"
	"dependencies/sources/libvorbis-1.3.6/lib/window.c"
)

include_directories(
	dependencies/sources/libogg-1.3.3/include
	dependencies/sources/libogg-1.3.3/linux_x86/include
	dependencies/sources/libvorbis-1.3.6/include
	dependencies/sources/libvorbis-1.3.6/lib
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

add_library(vorbis STATIC ${VORBIS_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(vorbis PRIVATE "-w")
endif()

#-------------------------------------------------------------------------------

project (vorbisfile)

file(GLOB VORBISFILE_SOURCES "dependencies/sources/libvorbis-1.3.6/lib/vorbisfile.c")

include_directories(
	dependencies/sources/libogg-1.3.3/include
	dependencies/sources/libogg-1.3.3/linux_x86/include
	dependencies/sources/libvorbis-1.3.6/include
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

add_library(vorbisfile STATIC ${VORBISFILE_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(vorbisfile PRIVATE "-w")
endif()

#-------------------------------------------------------------------------------

project (portaudio)

file(GLOB PORTAUDIO_OUTPUT_SOURCES
	"dependencies/sources/portaudio-v190600/src/common/pa_allocation.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_converters.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_cpuload.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_debugprint.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_dither.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_front.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_process.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_ringbuffer.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_stream.c"
	"dependencies/sources/portaudio-v190600/src/common/pa_trace.c"
	"dependencies/sources/portaudio-v190600/src/os/unix/pa_unix_hostapis.c"
	"dependencies/sources/portaudio-v190600/src/os/unix/pa_unix_util.c"
	"dependencies/sources/portaudio-v190600/src/hostapi/alsa/pa_linux_alsa.c"
)

include_directories(
	dependencies/sources/portaudio-v190600/src/common
	dependencies/sources/portaudio-v190600/include
	dependencies/sources/portaudio-v190600/src/os/unix
)

add_definitions(
	-DPA_LITTLE_ENDIAN
	-DPACKAGE_NAME=""
	-DPACKAGE_TARNAME=""
	-DPACKAGE_VERSION=""
	-DPACKAGE_STRING=""
	-DPACKAGE_BUGREPORT=""
	-DPACKAGE_URL=""
	-DSTDC_HEADERS=1
	-DHAVE_SYS_TYPES_H=1
	-DHAVE_SYS_STAT_H=1
	-DHAVE_STDLIB_H=1
	-DHAVE_STRING_H=1
	-DHAVE_MEMORY_H=1
	-DHAVE_STRINGS_H=1
	-DHAVE_INTTYPES_H=1
	-DHAVE_STDINT_H=1
	-DHAVE_UNISTD_H=1
	-DHAVE_DLFCN_H=1
	-DLT_OBJDIR=".libs/"
	-DHAVE_SYS_SOUNDCARD_H=1
	-DHAVE_LINUX_SOUNDCARD_H=1
	-DSIZEOF_SHORT=2
	-DSIZEOF_INT=4
	-DSIZEOF_LONG=8
	-DHAVE_CLOCK_GETTIME=1
	-DHAVE_NANOSLEEP=1
	-DPA_USE_ALSA=1
	-DPA_USE_JACK=0
	-DPA_USE_OSS=0
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(portaudio STATIC ${PORTAUDIO_OUTPUT_SOURCES})

#-------------------------------------------------------------------------------

project (oggdecoder)

file(GLOB OGG_DECODER_SOURCES "src/plugins/OggDecoder/*.cpp" "src/plugins/OggMetadata/OggMetadata.cpp")

include_directories(
	src/common
	dependencies/sources/libogg-1.3.3/include
	dependencies/sources/libvorbis-1.3.6/include
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(oggdecoder MODULE ${OGG_DECODER_SOURCES})
target_link_libraries(oggdecoder vorbisfile vorbis ogg)

#-------------------------------------------------------------------------------

project (nulloutput)

file(GLOB NULL_OUTPUT_SOURCES "src/plugins/NullOutput/*.cpp" "src/common/sha256.c")

include_directories(src/common)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nulloutput MODULE ${NULL_OUTPUT_SOURCES})

#-------------------------------------------------------------------------------

project (portaudiooutput)

file(GLOB PA_OUTPUT_SOURCES
	"src/plugins/PortAudioOutput/*.cpp"
	"src/common/sha256.c"
	"src/common/utf8.cpp"
)

include_directories(
	src/common
	dependencies/sources/portaudio-v190600/include
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(portaudiooutput MODULE ${PA_OUTPUT_SOURCES})
target_link_libraries(portaudiooutput portaudio asound)

#-------------------------------------------------------------------------------

project (napalm)

file(GLOB NAPALM_SOURCES "src/napalm/*.cpp")

#INCLUDE(FindPkgConfig)
#PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
include_directories(src/common dependencies/sources/libsamplerate-0.1.9/src)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(napalm MODULE ${NAPALM_SOURCES})
target_link_libraries(napalm pthread boost_system boost_filesystem samplerate)

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(napalm PRIVATE "-fvisibility=hidden")
endif()

#-------------------------------------------------------------------------------

project (t_napalm)

file(GLOB TNAPALM_SOURCES
	"src/interfaces/t-napalm/main.cpp"
	"src/external/Player.cpp"
)

include_directories(src/common src/external)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(t_napalm ${TNAPALM_SOURCES})
target_link_libraries(t_napalm boost_system boost_filesystem dl)

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(t_napalm PRIVATE "-fvisibility=hidden")
endif()
