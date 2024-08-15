cmake_minimum_required(VERSION 3.22)

if (APPLE)
	set (CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version" FORCE)
	set (CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
	set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()


project(AudiblePlanets)
set_property (DIRECTORY APPEND PROPERTY LABELS ${PROJECT_NAME})


add_subdirectory(JUCE)

foreach(module_name IN ITEMS gin gin_dsp gin_graphics gin_gui gin_plugin gin_simd)
	juce_add_module (
		"${CMAKE_CURRENT_LIST_DIR}/modules/gin/modules/${module_name}"
		)

	set_property (TARGET "${module_name}" APPEND PROPERTY LABELS Gin)
endforeach()

set(VersionString 1.1.5)
# also set this in PluginEditor.cpp -> showAboutInfo()

juce_add_plugin (${PROJECT_NAME}
				PRODUCT_NAME "Audible Planets" 
				VERSION ${VersionString}
				COMPANY_NAME "Void-Star"
				COMPANY_WEBSITE "https://void-star.com/"
    			ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
				FORMATS "VST3" "AU" "Standalone" "LV2"
				BUNDLE_ID com.void-star.audible-planets
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


if (APPLE)
	target_compile_options(${PROJECT_NAME} PRIVATE 
			-Wall -Wstrict-aliasing -Wunused-parameter -Wconditional-uninitialized -Woverloaded-virtual -Wreorder -Wconstant-conversion -Wbool-conversion -Wextra-semi 
			-Wunreachable-code -Winconsistent-missing-destructor-override -Wshift-sign-overflow -Wnullable-to-nonnull-conversion -Wuninitialized -Wno-missing-field-initializers 
			-Wno-ignored-qualifiers -Wno-missing-braces -Wno-char-subscripts -Wno-unused-private-field -fno-aligned-allocation -Wunused-private-field -Wunreachable-code 
			-Wenum-compare -Wshadow -Wfloat-conversion -Wshadow-uncaptured-local -Wshadow-field -Wsign-compare -Wdeprecated-this-capture -Wimplicit-float-conversion 
			-ffast-math -fno-finite-math-only -Wfloat-equal
#			"$<$<CONFIG:RELEASE>:-Werror>"
		)
endif ()

if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE 
			/wd26495 /wd4459 /wd4244 /wd4996 /wd4267 /wd4244
		)
    add_compile_definitions(MIPP_ALIGNED_LOADS)
endif ()

# Binary Data

set_property (DIRECTORY APPEND PROPERTY LABELS Assets)

file (GLOB_RECURSE source_files CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)

target_sources (${PROJECT_NAME} PRIVATE ${source_files})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX Source FILES ${source_files})

file(GLOB_RECURSE AssetFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.otf"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.json"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.wav"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.xml"
)

juce_add_binary_data(Assets SOURCES ${AssetFiles})

# Required for Linux happiness:
set_target_properties(Assets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)

target_link_libraries (${PROJECT_NAME} 
					PRIVATE
						gin
						gin_graphics
						gin_gui
						gin_dsp
						gin_plugin
						gin_simd
						juce::juce_events
						juce::juce_graphics
						juce::juce_gui_basics
						juce::juce_gui_extra
						juce::juce_core
						juce::juce_audio_basics

						$<$<PLATFORM_ID:Linux>:curl>
					PUBLIC
						Assets
						juce::juce_recommended_config_flags
						juce::juce_recommended_lto_flags
						juce::juce_recommended_warning_flags
					)

if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "NO")
endif()

#juce_generate_juce_header (${PROJECT_NAME})

get_property (debug_configs GLOBAL PROPERTY DEBUG_CONFIGURATIONS)

if (NOT debug_configs)
    set (debug_configs Debug)
endif ()

set (config_is_debug "$<IN_LIST:$<CONFIG>,${debug_configs}>")
set (config_is_release "$<NOT:${config_is_debug}>")

target_compile_definitions (${PROJECT_NAME} PRIVATE 
								JUCE_MODAL_LOOPS_PERMITTED=1
								JUCE_COREGRAPHICS_DRAW_ASYNC=1
								JUCE_VST3_CAN_REPLACE_VST2=0
								JUCE_WEB_BROWSER=0
								JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
							)

if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES
			XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] YES
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
            # XCODE_ATTRIBUTE_ARCHS '$(ARCHS_STANDARD)'
			)
endif()
