cmake_minimum_required(VERSION 3.14.5 FATAL_ERROR)

# Change project and application name to your own
set(MY_PROJECT_NAME navCAS)
set(MY_APP_NAME navCASApp)

#-----------------------------------------------------------------------------
# Set the language standard (MITK requires C++14)
#-----------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_EXTENSIONS 0)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
               STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#-----------------------------------------------------------------------------
# Superbuild Option - Not enabled by default
#-----------------------------------------------------------------------------

option(${MY_PROJECT_NAME}_USE_SUPERBUILD "Build ${MY_PROJECT_NAME} and the projects it depends on via SuperBuild.cmake." OFF)

if(${MY_PROJECT_NAME}_USE_SUPERBUILD)
  project(${MY_PROJECT_NAME}-superbuild)
  set(${MY_PROJECT_NAME}_SOURCE_DIR ${PROJECT_SOURCE_DIR})
  set(${MY_PROJECT_NAME}_BINARY_DIR ${PROJECT_BINARY_DIR})
else()
  project(${MY_PROJECT_NAME})
endif()

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

set(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  CMP0020 # NEW: Automatically link Qt executables to qtmain target on Windows.
  CMP0028 # NEW: Double colon in target name means ALIAS or IMPORTED target.
  )
foreach(policy ${project_policies})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()


#-----------------------------------------------------------------------------
# Check miminum compiler versions
#-----------------------------------------------------------------------------

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # require at least gcc 4.9 as provided by ppa:ubuntu-toolchain-r/test for Ubuntu 14.04
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    message(FATAL_ERROR "GCC version must be at least 4.9
If you are using Ubuntu 14.04, you can easily install gcc and g++ 4.9 (or any later version available) in addition to your version ${CMAKE_CXX_COMPILER_VERSION}:
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo apt-get update
  sudo apt-get install gcc-4.9 g++-4.9
Make sure to explicitly specify these compilers when configuring MITK:
  CMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc-4.9
  CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++-4.9
For more information on the proposed PPA see the Toolchain Updates section of https://wiki.ubuntu.com/ToolChain.")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # require at least clang 3.4 as provided by Ubuntu 12.04
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
    message(FATAL_ERROR "Clang version must be at least 3.4")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # require at least clang 5.0
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "Apple Clang version must be at least 5.0")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # require at least Visual Studio 2013
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0.40629.0)
    message(FATAL_ERROR "Microsoft Visual Studio 2013 Update 5 or newer required (MSVC 18.0.40629.0)")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang (Linux or Apple), GCC and MSVC.")
endif()


#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH
  ${${MY_PROJECT_NAME}_SOURCE_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------

include(MacroEmptyExternalProject)

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

foreach(type LIBRARY RUNTIME ARCHIVE)
  set(output_dir ${${MY_PROJECT_NAME}_BINARY_DIR}/bin)
  set(CMAKE_${type}_OUTPUT_DIRECTORY ${output_dir} CACHE INTERNAL "Single output directory for building all libraries.")
  mark_as_advanced(CMAKE_${type}_OUTPUT_DIRECTORY)
endforeach()

#-----------------------------------------------------------------------------
# Additional Options (also shown during superbuild)
#-----------------------------------------------------------------------------

option(BUILD_SHARED_LIBS "Build ${MY_PROJECT_NAME} with shared libraries" ON)
option(WITH_COVERAGE "Enable/Disable coverage" OFF)
option(BUILD_TESTING "Test the project" ON)

option(${MY_PROJECT_NAME}_BUILD_ALL_PLUGINS "Build all ${MY_PROJECT_NAME} plugins" OFF)
option(${MY_PROJECT_NAME}_BUILD_ALL_APPS "Build all ${MY_PROJECT_NAME} applications" OFF)

mark_as_advanced(${MY_PROJECT_NAME}_INSTALL_RPATH_RELATIVE
                 ${MY_PROJECT_NAME}_BUILD_ALL_PLUGINS
                 ${MY_PROJECT_NAME}_BUILD_ALL_APPS
                 )

#-----------------------------------------------------------------------------
# Superbuild script
#-----------------------------------------------------------------------------

if(${MY_PROJECT_NAME}_USE_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#*****************************************************************************
#**************************** END OF SUPERBUILD ****************************
#*****************************************************************************

#-----------------------------------------------------------------------------
# Prerequesites
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# --- Atracsys ---
#-----------------------------------------------------------------------------


IF(WIN32)
  # On Windows, we look for the installation folder in the registry
  GET_FILENAME_COMPONENT(ATRACSYS_SDK_INSTALL_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Atracsys\\spryTrack;Root]" ABSOLUTE CACHE)
ELSE()
  # On Linux, we assume that the installation folder is on directory below.
  # SET(ATRACSYS_SDK_INSTALL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/spryTrack_sdk_v4)
  
  find_path(ATRACSYS_SDK_INSTALL_PATH DOC "Atracsys sdk path" REQUIRED)
ENDIF()

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
	"${ATRACSYS_SDK_INSTALL_PATH}/include"
)

LINK_DIRECTORIES(
	"${ATRACSYS_SDK_INSTALL_PATH}/lib"
)

# Atracsys SDK API
# This will be only used to display the headers in Visual Studio
SET( api_SRCS
	"${ATRACSYS_SDK_INSTALL_PATH}/include/ftkErrors.h"
	"${ATRACSYS_SDK_INSTALL_PATH}/include/ftkTypes.h"
	"${ATRACSYS_SDK_INSTALL_PATH}/include/ftkErrors.h"
	"${ATRACSYS_SDK_INSTALL_PATH}/include/ftkInterface.h"
	"${ATRACSYS_SDK_INSTALL_PATH}/include/ftkPlatform.h"
	"${ATRACSYS_SDK_INSTALL_PATH}/include/ftkOptions.h"
)

# Helpers dedicated to the samples
SET( additional_SRCS
	"${ATRACSYS_SDK_INSTALL_PATH}/samples/helpers.hpp"
	"${ATRACSYS_SDK_INSTALL_PATH}/samples/geometryHelper.hpp"
)

#SET(ENV{LD_LIBRARY_PATH}
#  "/home/axel/Desktop/Atracsys/SDK/spryTrack_sdk_v2/lib"
#  )

#SET( LD_LIBRARY_PATH
#  "${ATRACSYS_SDK_INSTALL_PATH}/lib"
#)

#SET(ENV{LD_LIBRARY_PATH}
#  "${ATRACSYS_SDK_INSTALL_PATH}/lib"
#)





IF (WIN32)
	SET( additional_SRCS ${additional_SRCS} "${ATRACSYS_SDK_INSTALL_PATH}/samples/helpers_windows.cpp" )
ELSE (WIN32)
	SET( additional_SRCS ${additional_SRCS} "${ATRACSYS_SDK_INSTALL_PATH}/samples/helpers_linux.cpp" )
ENDIF (WIN32)

IF( "${CMAKE_SIZEOF_VOID_P}" EQUAL "8" )
    SET( ARCH 64 )
ELSE( "${CMAKE_SIZEOF_VOID_P}" EQUAL "8" )
    SET( ARCH 32 )
ENDIF( "${CMAKE_SIZEOF_VOID_P}" EQUAL "8" )
SET( LIBS ${LIBS} fusionTrack${ARCH} )


IF (WIN32)
  # On Windows, we use the delay loading of DLLs along with the function
  # SetDllDirectory() to load dlls outside of the Executable folder.
  SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " /DELAYLOAD:fusionTrack${ARCH}.dll")
  ADD_DEFINITIONS(-DFORCED_DEVICE_DLL_PATH="${ATRACSYS_SDK_INSTALL_PATH}/bin")
  SET( LIBS ${LIBS} delayimp )

  SET( LIBS ${LIBS} Winmm )
	IF (MSVC_IDE)
	  SET( LIBS ${LIBS} optimized MSVCRT )
	  SET( LIBS ${LIBS} debug MSVCRTD )
  ENDIF (MSVC_IDE)
ENDIF (WIN32)

FIND_PACKAGE (Threads)
SET( LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT} )


# Copy geometry files to bin directory
configure_file(${ATRACSYS_SDK_INSTALL_PATH}/data/geometry001.ini
    ./data/geometry001.ini COPYONLY)
configure_file(${ATRACSYS_SDK_INSTALL_PATH}/data/geometry002.ini
    ./data/geometry002.ini COPYONLY)
configure_file(${ATRACSYS_SDK_INSTALL_PATH}/data/geometry003.ini
    ./data/geometry003.ini COPYONLY)
configure_file(${ATRACSYS_SDK_INSTALL_PATH}/data/geometry004.ini
    ./data/geometry004.ini COPYONLY)

# Copy probe geometries (only if they don't exist already)
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/data/geometry_123_probe.ini)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Resources/geometry_123_probe.ini
      ./data/geometry_123_probe.ini COPYONLY)
endif()
if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/Resources/geometry_124_temporal_probe.ini)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Resources/geometry_124_temporal_probe.ini
      ./data/geometry_124_temporal_probe.ini COPYONLY)
endif()
	
# Copy helper code implementations from the Atracsys API
if (WIN32)
  configure_file(${ATRACSYS_SDK_INSTALL_PATH}/samples/helpers_windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers_windows.cpp)
else()
    configure_file(${ATRACSYS_SDK_INSTALL_PATH}/samples/helpers_linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers_linux.cpp)
endif()


# Create patients directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/patients)

# Copy needed dll's for windows
IF (WIN32)
  configure_file(${ATRACSYS_SDK_INSTALL_PATH}/bin/fusionTrack64.dll
    ${CMAKE_CURRENT_BINARY_DIR}/bin/Release/fusionTrack64.dll COPYONLY)
  configure_file(${ATRACSYS_SDK_INSTALL_PATH}/bin/device64.dll
    ${CMAKE_CURRENT_BINARY_DIR}/bin/Release/device64.dll COPYONLY)
  configure_file(${ATRACSYS_SDK_INSTALL_PATH}/bin/libusb-1.0.dll
    ${CMAKE_CURRENT_BINARY_DIR}/bin/Release/libusb-1.0.dll COPYONLY)
  configure_file(${ATRACSYS_SDK_INSTALL_PATH}/bin/fileSignature64.dll
    ${CMAKE_CURRENT_BINARY_DIR}/bin/Release/fileSignature64.dll COPYONLY)
  configure_file(${ATRACSYS_SDK_INSTALL_PATH}/bin/dump2cams64.dll
    ${CMAKE_CURRENT_BINARY_DIR}/bin/Release/dump2cams64.dll COPYONLY)
ENDIF (WIN32)

#ADD_EXECUTABLE( navigation "./src/main.cpp" ${api_SRCS} ${additional_SRCS} )
#TARGET_LINK_LIBRARIES( navigation ${LIBS} ${VTK_LIBRARIES})



#*****************************************************************************
#****************************** END OF ATRACSYS ******************************
#*****************************************************************************

# openmp
find_package(OpenMP REQUIRED)

# Look for optional Doxygen package
find_package(Doxygen)

find_package(Qt5WebEngine REQUIRED)

# SQLite
#find_package (SQLite3)
#if (SQLITE3_FOUND)
#  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIRS})
#  LINK_DIRECTORIES(${SQLite3_LIBRARIES})

# #target_link_libraries (${OUT_TARGET} sqlite3)

#add_custom_target(build-time-make-directory ALL
#    COMMAND ${CMAKE_COMMAND} -E make_directory "./patients")

#endif (SQLITE3_FOUND)

set(${PROJECT_NAME}_MODULES_PACKAGE_DEPENDS_DIR "${PROJECT_SOURCE_DIR}/CMake/PackageDepends")
list(APPEND MODULES_PACKAGE_DEPENDS_DIRS ${${PROJECT_NAME}_MODULES_PACKAGE_DEPENDS_DIR})

# MITK
find_package(MITK 2018.04.99 REQUIRED)

if(COMMAND mitkFunctionCheckMitkCompatibility)
  mitkFunctionCheckMitkCompatibility(VERSIONS MITK_VERSION_PLUGIN_SYSTEM 1 REQUIRED)
else()
  message(SEND_ERROR "Your MITK version is too old. Please use Git hash b86bf28 or newer")
endif()

link_directories(${MITK_LINK_DIRECTORIES})






#-----------------------------------------------------------------------------
# CMake Function(s) and Macro(s)
#-----------------------------------------------------------------------------

set(CMAKE_MODULE_PATH
  ${MITK_SOURCE_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

include(mitkFunctionCheckCompilerFlags)
include(mitkFunctionGetGccVersion)
include(mitkFunctionGetVersion)

#-----------------------------------------------------------------------------
# Set project specific options and variables (NOT available during superbuild)
#-----------------------------------------------------------------------------

set(${PROJECT_NAME}_VERSION_MAJOR "0")
set(${PROJECT_NAME}_VERSION_MINOR "1")
set(${PROJECT_NAME}_VERSION_PATCH "1")
set(${PROJECT_NAME}_VERSION_STRING "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")


# Ask the user if a console window should be shown with the applications
option(MITK_SHOW_CONSOLE_WINDOW "Use this to enable or disable the console window when starting GUI Applications" ON)
mark_as_advanced(MITK_SHOW_CONSOLE_WINDOW)

if(NOT UNIX AND NOT MINGW)
  set(MITK_WIN32_FORCE_STATIC "STATIC")
endif()


#-----------------------------------------------------------------------------
# Get project version info
#-----------------------------------------------------------------------------

mitkFunctionGetVersion(${PROJECT_SOURCE_DIR} ${PROJECT_NAME})

#-----------------------------------------------------------------------------
# Documentation
#-----------------------------------------------------------------------------

if(DOXYGEN_FOUND)
  add_subdirectory(Documentation)
endif()

#-----------------------------------------------------------------------------
# Installation preparation
#
# These should be set before any MITK install macros are used
#-----------------------------------------------------------------------------

# on Mac OSX all CTK plugins get copied into every
# application bundle (.app directory) specified here
set(MACOSX_BUNDLE_NAMES)
if(APPLE)
  list(APPEND MACOSX_BUNDLE_NAMES ${MY_APP_NAME})
endif(APPLE)

#-----------------------------------------------------------------------------
# Set symbol visibility Flags
#-----------------------------------------------------------------------------

# MinGW does not export all symbols automatically, so no need to set flags
if(CMAKE_COMPILER_IS_GNUCXX)
  # The MITK module build system does not yet support default hidden visibility
  set(VISIBILITY_CXX_FLAGS ) # "-fvisibility=hidden -fvisibility-inlines-hidden")
endif()

#-----------------------------------------------------------------------------
# Set coverage Flags
#-----------------------------------------------------------------------------

if(WITH_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(coverage_flags "-g -fprofile-arcs -ftest-coverage -O0 -DNDEBUG")
    set(COVERAGE_CXX_FLAGS ${coverage_flags})
    set(COVERAGE_C_FLAGS ${coverage_flags})
  endif()
endif()

#-----------------------------------------------------------------------------
# Project C/CXX Flags
#-----------------------------------------------------------------------------

set(${PROJECT_NAME}_C_FLAGS "${MITK_C_FLAGS} ${COVERAGE_C_FLAGS}")
set(${PROJECT_NAME}_C_FLAGS_DEBUG ${MITK_C_FLAGS_DEBUG})
set(${PROJECT_NAME}_C_FLAGS_RELEASE ${MITK_C_FLAGS_RELEASE})
set(${PROJECT_NAME}_CXX_FLAGS "${MITK_CXX_FLAGS} ${VISIBILITY_CXX_FLAGS} ${COVERAGE_CXX_FLAGS}")
set(${PROJECT_NAME}_CXX_FLAGS_DEBUG ${MITK_CXX_FLAGS_DEBUG})
set(${PROJECT_NAME}_CXX_FLAGS_RELEASE ${MITK_CXX_FLAGS_RELEASE})

set(${PROJECT_NAME}_EXE_LINKER_FLAGS ${MITK_EXE_LINKER_FLAGS})
set(${PROJECT_NAME}_SHARED_LINKER_FLAGS ${MITK_SHARED_LINKER_FLAGS})
set(${PROJECT_NAME}_MODULE_LINKER_FLAGS ${MITK_MODULE_LINKER_FLAGS})


#-----------------------------------------------------------------------------
# Set C/CXX Flags
#-----------------------------------------------------------------------------

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${PROJECT_NAME}_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${${PROJECT_NAME}_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${${PROJECT_NAME}_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${${PROJECT_NAME}_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${${PROJECT_NAME}_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${${PROJECT_NAME}_CXX_FLAGS_RELEASE}")

set(CMAKE_EXE_LINKER_FLAGS ${${PROJECT_NAME}_EXE_LINKER_FLAGS})
set(CMAKE_SHARED_LINKER_FLAGS ${${PROJECT_NAME}_SHARED_LINKER_FLAGS})
set(CMAKE_MODULE_LINKER_FLAGS ${${PROJECT_NAME}_MODULE_LINKER_FLAGS})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------

if(BUILD_TESTING)
  enable_testing()
  include(CTest)
  mark_as_advanced(TCL_TCLSH DART_ROOT)

  # Setup file for setting custom ctest vars
  configure_file(
    CMake/CTestCustom.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
    @ONLY
    )

  # Configuration for the CMake-generated test driver
  set(CMAKE_TESTDRIVER_EXTRA_INCLUDES "#include <stdexcept>")
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "
    try
      {")
  set(CMAKE_TESTDRIVER_AFTER_TESTMAIN " }
      catch( std::exception & excp )
        {
        fprintf(stderr,\"%s\\n\",excp.what());
        return EXIT_FAILURE;
        }
      catch( ... )
        {
        printf(\"Exception caught in the test driver\\n\");
        return EXIT_FAILURE;
        }
      ")
endif()

#-----------------------------------------------------------------------------
# ${MY_PROJECT_NAME}_SUPERBUILD_BINARY_DIR
#-----------------------------------------------------------------------------

# If ${MY_PROJECT_NAME}_SUPERBUILD_BINARY_DIR isn't defined, it means this project is
# *NOT* build using Superbuild. In that specific case, ${MY_PROJECT_NAME}_SUPERBUILD_BINARY_DIR
# should default to PROJECT_BINARY_DIR
if(NOT DEFINED ${PROJECT_NAME}_SUPERBUILD_BINARY_DIR)
  set(${PROJECT_NAME}_SUPERBUILD_BINARY_DIR ${PROJECT_BINARY_DIR})
endif()

#-----------------------------------------------------------------------------
# Qt support
#-----------------------------------------------------------------------------

if(MITK_USE_Qt5)
  set(QT_QMAKE_EXECUTABLE ${MITK_QMAKE_EXECUTABLE})
  add_definitions(-DQWT_DLL)
endif()

#-----------------------------------------------------------------------------
# MITK modules
#-----------------------------------------------------------------------------

# This project's directory holding module config files
set(${PROJECT_NAME}_MODULES_CONF_DIR "${PROJECT_BINARY_DIR}/${MODULES_CONF_DIRNAME}")

# Append this projects's module config directory to the global list
# (This is used to get include directories for the <module_name>Exports.h files right)
list(APPEND MODULES_CONF_DIRS ${${PROJECT_NAME}_MODULES_CONF_DIR})

# Clean the modulesConf directory. This ensures that modules are sorted
# according to their dependencies in the Modules/CMakeLists.txt file
file(GLOB _modules_conf_files ${${PROJECT_NAME}_MODULES_CONF_DIR}/*.cmake)
if(_modules_conf_files)
  file(REMOVE ${_modules_conf_files})
endif()

add_subdirectory(Modules)

#-----------------------------------------------------------------------------
# CTK plugins
#-----------------------------------------------------------------------------

# The CMake code in this section *must* be in the top-level CMakeLists.txt file

macro(GetMyTargetLibraries all_target_libraries varname)
  set(re_ctkplugin "^navcas_[a-zA-Z0-9_]+$")
  set(_tmp_list)
  list(APPEND _tmp_list ${all_target_libraries})
  ctkMacroListFilter(_tmp_list re_ctkplugin OUTPUT_VARIABLE ${varname})
endmacro()

# Get infos about application directories and build options
include("${CMAKE_CURRENT_SOURCE_DIR}/Apps/Apps.cmake")
set(_apps_fullpath )
foreach(_app ${PROJECT_APPS})
  list(APPEND _apps_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/${_app}")
endforeach()

include(${CMAKE_CURRENT_SOURCE_DIR}/Plugins/Plugins.cmake)
ctkMacroSetupPlugins(${PROJECT_PLUGINS}
                     APPS ${_apps_fullpath}
                     BUILD_OPTION_PREFIX ${PROJECT_NAME}_
                     BUILD_ALL ${${PROJECT_NAME}_BUILD_ALL_PLUGINS})

#-----------------------------------------------------------------------------
# Add subdirectories
#-----------------------------------------------------------------------------

add_subdirectory(Apps)

#-----------------------------------------------------------------------------
# Installation
#-----------------------------------------------------------------------------

# set MITK cpack variables
include(navcasSetupCPack)

# Customize CPack variables for this project
include(CPackSetup)

list(APPEND CPACK_CREATE_DESKTOP_LINKS "${MY_APP_NAME}")

configure_file(${MITK_SOURCE_DIR}/MITKCPackOptions.cmake.in
               ${PROJECT_BINARY_DIR}/${PROJECT_NAME}CPackOptions.cmake @ONLY)
set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}CPackOptions.cmake")

# include CPack model once all variables are set
include(CPack)

# Additional installation rules
include(mitkInstallRules)

if(WIN32)
  install(FILES "${PROJECT_SOURCE_DIR}/Externals/openssl/win64/ssleay32.dll" DESTINATION "bin" CONFIGURATIONS Release)
  install(FILES "${PROJECT_SOURCE_DIR}/Externals/openssl/win64/libeay32.dll" DESTINATION "bin" CONFIGURATIONS Release)
endif()

#-----------------------------------------------------------------------------
# Last configuration steps
#-----------------------------------------------------------------------------

# If we are under Windows, create two batch files which correctly
# set up the environment for the application and for Visual Studio
if(WIN32)
  include(mitkFunctionCreateWindowsBatchScript)

  set(VS_SOLUTION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.sln")
  foreach(VS_BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript("${PROJECT_SOURCE_DIR}/CMake/StartVS.bat.in"
      ${PROJECT_BINARY_DIR}/StartVS_${VS_BUILD_TYPE}.bat
      ${VS_BUILD_TYPE})
  endforeach()
endif(WIN32)
