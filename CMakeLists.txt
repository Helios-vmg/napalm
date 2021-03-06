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

project (flac)

file(GLOB FLAC_SOURCES
	"dependencies/sources/flac-1.3.2/src/libFLAC/bitmath.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/bitreader.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/bitwriter.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/cpu.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/crc.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/fixed.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/fixed_intrin_sse2.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/fixed_intrin_ssse3.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/float.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/format.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/lpc.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/lpc_intrin_avx2.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/lpc_intrin_sse.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/lpc_intrin_sse2.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/lpc_intrin_sse41.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/md5.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/memory.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/metadata_iterators.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/metadata_object.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/ogg_decoder_aspect.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/ogg_encoder_aspect.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/ogg_helper.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/ogg_mapping.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/stream_decoder.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/stream_encoder.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/stream_encoder_framing.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/stream_encoder_intrin_avx2.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/stream_encoder_intrin_sse2.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/stream_encoder_intrin_ssse3.c"
	"dependencies/sources/flac-1.3.2/src/libFLAC/window.c"
)

if (WIN32)
	set(FLAC_SOURCES ${FLAC_SOURCES}
		"dependencies/sources/flac-1.3.2/src/libFLAC/windows_unicode_filenames.c"
	)
endif (WIN32)

include_directories(
	dependencies/sources/flac-1.3.2/src/libFLAC/include
	dependencies/sources/flac-1.3.2/include
	dependencies/sources/libogg-1.3.3/include
)

add_definitions(
	-DHAVE_LROUND
	-DFLAC__HAS_OGG
	-DFLAC__ALIGN_MALLOC_DATA
	-DFLAC__NO_DLL
	-DFLAC__LPC_UNROLLED_FILTER_LOOPS
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

add_library(flac STATIC ${FLAC_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(flac PRIVATE "-w")
endif()

#-------------------------------------------------------------------------------

project (flacpp)

file(GLOB FLACPP_SOURCES
	"dependencies/sources/flac-1.3.2/src/libFLAC++/metadata.cpp"
	"dependencies/sources/flac-1.3.2/src/libFLAC++/stream_decoder.cpp"
	"dependencies/sources/flac-1.3.2/src/libFLAC++/stream_encoder.cpp"
)

include_directories(
	dependencies/sources/flac-1.3.2/include
)

add_definitions(
	-DFLAC__NO_DLL
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE True)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(flacpp STATIC ${FLACPP_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(flacpp PRIVATE "-w")
endif()

#-------------------------------------------------------------------------------

project (cue)

file(GLOB CUE_SOURCES
	"dependencies/sources/libcue-2.2.1/src/cd.c"
	"dependencies/sources/libcue-2.2.1/src/cdtext.c"
	"dependencies/sources/libcue-2.2.1/src/cue_parser.c"
	"dependencies/sources/libcue-2.2.1/src/cue_scanner.c"
	"dependencies/sources/libcue-2.2.1/src/rem.c"
	"dependencies/sources/libcue-2.2.1/src/time.c"
)

include_directories(
	dependencies/sources/libcue-2.2.1/include
)

add_definitions(
	-DLIBCUE_EXPORTS
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

add_library(cue STATIC ${CUE_SOURCES})

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(cue PRIVATE "-w")
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
)

if (WIN32)
	set(PORTAUDIO_OUTPUT_SOURCES ${PORTAUDIO_OUTPUT_SOURCES}
		"dependencies/sources/portaudio-v190600/src/os/win/pa_win_coinitialize.c"
		"dependencies/sources/portaudio-v190600/src/os/win/pa_win_hostapis.c"
		"dependencies/sources/portaudio-v190600/src/os/win/pa_win_util.c"
		"dependencies/sources/portaudio-v190600/src/os/win/pa_win_waveformat.c"
		"dependencies/sources/portaudio-v190600/src/os/win/pa_win_wdmks_utils.c"
		"dependencies/sources/portaudio-v190600/src/os/win/pa_x86_plain_converters.c"
	)
endif (WIN32)

if (UNIX)
	set(PORTAUDIO_OUTPUT_SOURCES ${PORTAUDIO_OUTPUT_SOURCES}
		"dependencies/sources/portaudio-v190600/src/os/unix/pa_unix_hostapis.c"
		"dependencies/sources/portaudio-v190600/src/os/unix/pa_unix_util.c"
		"dependencies/sources/portaudio-v190600/src/hostapi/alsa/pa_linux_alsa.c"
	)
endif (UNIX)

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

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(portaudio PRIVATE "-w")
endif()

#-------------------------------------------------------------------------------

project (oggdecoder)

file(GLOB OGG_DECODER_SOURCES
	"src/plugins/OggDecoder/*.cpp"
	"src/common/base64.cpp"
	"src/plugins/OggMetadata/OggMetadata.cpp"
)

include_directories(
	mingw-std-threads
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

project (flacdecoder)

file(GLOB FLAC_DECODER_SOURCES
	"src/plugins/FlacDecoder/*.cpp"
	"src/common/base64.cpp"
	"src/plugins/OggMetadata/OggMetadata.cpp"
)

include_directories(
	mingw-std-threads
	src/common
	dependencies/sources/flac-1.3.2/include
	dependencies/sources/libogg-1.3.3/include
	dependencies/sources/libvorbis-1.3.6/include
	dependencies/sources/libcue-2.2.1/include
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(flacdecoder MODULE ${FLAC_DECODER_SOURCES})
target_link_libraries(flacdecoder cue flacpp flac vorbisfile vorbis ogg)

#-------------------------------------------------------------------------------

project (nulloutput)

file(GLOB NULL_OUTPUT_SOURCES
	"src/plugins/NullOutput/*.cpp"
	"src/common/sha256.c"
)

include_directories(
	mingw-std-threads
	src/common
)

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
	mingw-std-threads
	src/common
	dependencies/sources/portaudio-v190600/include
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(portaudiooutput MODULE ${PA_OUTPUT_SOURCES})
if (WIN32)
	target_link_libraries(portaudiooutput portaudio winmm)
endif (WIN32)
if (UNIX)
	target_link_libraries(portaudiooutput portaudio asound)
endif (UNIX)

#-------------------------------------------------------------------------------

project (napalm)

file(GLOB NAPALM_SOURCES "src/napalm/*.cpp")

#INCLUDE(FindPkgConfig)
#PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
include_directories(
	mingw-std-threads
	src/common
	dependencies/sources/libsamplerate-0.1.9/src
)

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

include_directories(
	mingw-std-threads
	src/common
	src/external
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(t_napalm ${TNAPALM_SOURCES})
target_link_libraries(t_napalm boost_system boost_filesystem dl)

if (CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(t_napalm PRIVATE "-fvisibility=hidden")
endif()
