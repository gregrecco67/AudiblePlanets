cmake_minimum_required(VERSION 3.22)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set(PROJECT_NAME "AudiblePlanets")
set(PRODUCT_NAME "Audible Planets")
set(COMPANY_NAME "Void-Star")
set(BUNDLE_ID "com.void-star.audible-planets")
set(MAJOR_VERSION 1)
set(MINOR_VERSION 2)
set(PATCH_LEVEL 4)
set(CURRENT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL}")
set(FORMATS VST3) # Standalone AU LV2

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "CMAKE_BUILD_TYPE unspecified; picking Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
endif ()

# Color our warnings and errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif ()

project(${PROJECT_NAME})

add_subdirectory(modules/JUCE)
foreach(module_name IN ITEMS gin gin_dsp gin_graphics gin_gui gin_plugin gin_simd)
	juce_add_module ("${CMAKE_CURRENT_LIST_DIR}/modules/Gin/modules/${module_name}")
	set_property (TARGET "${module_name}" APPEND PROPERTY LABELS Gin)
endforeach()

#set(VersionString 1.2.3)
add_definitions( -DVERSION_STRING="${CURRENT_VERSION}" )
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")

juce_add_plugin (${PROJECT_NAME}
				PRODUCT_NAME ${PRODUCT_NAME}
				VERSION ${CURRENT_VERSION}
				COMPANY_NAME ${COMPANY_NAME}
				COMPANY_WEBSITE "https://void-star.com/"
    			ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
				FORMATS "${FORMATS}"
				BUNDLE_ID "${BUNDLE_ID}"
				DESCRIPTION "An expressive, quasi-Ptolemaic semi-modular synthesizer."
				PLUGIN_MANUFACTURER_CODE "Vdst"
				PLUGIN_CODE "APts" 
				AU_MAIN_TYPE "kAudioUnitType_MusicDevice"
				IS_SYNTH TRUE
				NEEDS_MIDI_INPUT TRUE
				NEEDS_MIDI_OUTPUT FALSE
				COPY_PLUGIN_AFTER_BUILD TRUE
				LV2URI "urn:void-star:audibleplanets:${VersionString}"
				JUCE_DONT_DECLARE_PROJECTINFO TRUE
			)

if (MSVC)
	target_compile_options(${PROJECT_NAME} INTERFACE $<$<CONFIG:RELEASE>:/fp:fast>)
else ()
	target_compile_options(${PROJECT_NAME} INTERFACE $<$<CONFIG:RELEASE>:-Ofast>)
	target_compile_options(${PROJECT_NAME} INTERFACE $<$<CONFIG:RelWithDebInfo>:-Ofast>)
endif ()

# Tell MSVC to properly report what c++ version is being used
if (MSVC)
	target_compile_options(${PROJECT_NAME} INTERFACE /Zc:__cplusplus)
endif ()

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)			

if (APPLE)
	target_compile_options(${PROJECT_NAME} PRIVATE 
			-Wall -Wstrict-aliasing -Wunused-parameter -Wconditional-uninitialized -Woverloaded-virtual -Wreorder -Wconstant-conversion -Wbool-conversion -Wextra-semi 
			-Wunreachable-code -Winconsistent-missing-destructor-override -Wshift-sign-overflow -Wnullable-to-nonnull-conversion -Wuninitialized -Wno-missing-field-initializers 
			-Wno-ignored-qualifiers -Wno-missing-braces -Wno-char-subscripts -Wno-unused-private-field -fno-aligned-allocation -Wunused-private-field -Wunreachable-code 
			-Wenum-compare -Wshadow -Wno-float-conversion -Wshadow-uncaptured-local -Wshadow-field -Wsign-compare -Wdeprecated-this-capture -Wno-implicit-float-conversion 
			-ffast-math -fno-finite-math-only -Wfloat-equal
#			"$<$<CONFIG:RELEASE>:-Werror>"
		)
	add_definitions( -DUSE_NEON )
endif ()

if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE 
            /wd26495 /wd4459 /wd4244 /wd4996 /wd4267 /wd4244
		)
    add_compile_definitions(MIPP_ALIGNED_LOADS USE_SSE)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_compile_definitions(USE_SSE)
endif ()

file (GLOB_RECURSE source_files CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/MTS-ESP/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/hiir/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/ADAAsrc/polylogarithm/Li2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/MTS-ESP/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/MTS-ESP/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/hiir/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/hiir/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/ADAAsrc/polylogarithm/Li2.*
	${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/ADAAsrc/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party/ADAAsrc/ADAA/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/*.hpp
	)

target_sources (${PROJECT_NAME} PRIVATE ${source_files})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX Source FILES ${source_files})
target_include_directories(${PROJECT_NAME} PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}/Source"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/UI"
		"${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party"
	)

set_property (DIRECTORY APPEND PROPERTY LABELS Assets)
file(GLOB_RECURSE AssetFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.otf"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.json"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.wav"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.xml"
)
juce_add_binary_data(Assets SOURCES ${AssetFiles})
set_target_properties(Assets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries (${PROJECT_NAME} 
					PRIVATE
						gin
						gin_graphics
						gin_gui
						gin_dsp
						gin_plugin
						gin_simd
						juce::juce_core
						juce::juce_dsp
						juce::juce_events
						juce::juce_graphics
						juce::juce_gui_basics
						juce::juce_gui_extra
						juce::juce_audio_basics
						$<$<PLATFORM_ID:Linux>:curl>
					PUBLIC
						Assets
						juce::juce_recommended_config_flags
						juce::juce_recommended_lto_flags
						#juce::juce_recommended_warning_flags
					)

if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "NO")
endif()

target_compile_definitions (${PROJECT_NAME} PRIVATE 
								JUCE_MODAL_LOOPS_PERMITTED=1
								JUCE_COREGRAPHICS_DRAW_ASYNC=1
								JUCE_VST3_CAN_REPLACE_VST2=0
								JUCE_WEB_BROWSER=0
								JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
							)

foreach (target ${FORMATS} "All")
    if (TARGET ${PROJECT_NAME}_${target})
        set_target_properties(${PROJECT_NAME}_${target} PROPERTIES
            # Tuck the actual plugin targets into a folder where they won't bother us
            FOLDER "Targets"
            # Let us build the target in Xcode
            XCODE_GENERATE_SCHEME ON)

        # Set the default executable that Xcode will open on build
        # Note: you must manually build the AudioPluginHost.xcodeproj in the JUCE subdir
        if ((NOT target STREQUAL "All") AND (NOT target STREQUAL "Standalone"))
            set_target_properties(${PROJECT_NAME}_${target} PROPERTIES
                XCODE_SCHEME_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/extras/AudioPluginHost/Builds/MacOSX/build/Debug/AudioPluginHost.app")
        endif ()
    endif ()
endforeach ()


if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES
			XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] YES
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
			)
endif()

set (env_file "${PROJECT_SOURCE_DIR}/.env")
message ("Writing ENV file for CI: ${env_file}")
# the first call truncates, the rest append
file(WRITE  "${env_file}" "PROJECT_NAME=${PROJECT_NAME}\n")
file(APPEND "${env_file}" "PRODUCT_NAME=${PRODUCT_NAME}\n")
file(APPEND "${env_file}" "VERSION=${CURRENT_VERSION}\n")
file(APPEND "${env_file}" "MAJOR_VERSION=${MAJOR_VERSION}\n")
file(APPEND "${env_file}" "MINOR_VERSION=${MINOR_VERSION}\n")
file(APPEND "${env_file}" "PATCH_LEVEL=${PATCH_LEVEL}\n")
file(APPEND "${env_file}" "BUNDLE_ID=${BUNDLE_ID}\n")
file(APPEND "${env_file}" "COMPANY_NAME=${COMPANY_NAME}\n")
