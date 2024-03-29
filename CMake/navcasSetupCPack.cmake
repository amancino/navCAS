#
# First, set the generator variable
#
if(NOT CPACK_GENERATOR)
  if(WIN32)
    set(CPACK_GENERATOR ZIP)
  else()
    if(APPLE)
      set(CPACK_GENERATOR DragNDrop)
    else()
      set(CPACK_GENERATOR TGZ)
    endif()
  endif()
endif(NOT CPACK_GENERATOR)

# Set Redistributable information for windows
if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  include(mitkFunctionGetMSVCVersion)
  mitkFunctionGetMSVCVersion()
  set(CPACK_VISUAL_STUDIO_VERSION_MAJOR "${VISUAL_STUDIO_VERSION_MAJOR}")
  set(CPACK_VISUAL_STUDIO_PRODUCT_NAME "${VISUAL_STUDIO_PRODUCT_NAME}")
  set(CPACK_LIBRARY_ARCHITECTURE "${CMAKE_LIBRARY_ARCHITECTURE}")
  set(CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE "" CACHE FILEPATH "Path to the appropriate Microsoft Visual Studio Redistributable")
endif()

if(EXISTS ${CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE} )
  install(PROGRAMS ${CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE}
          DESTINATION thirdpartyinstallers)

  get_filename_component(CPACK_REDISTRIBUTABLE_FILE_NAME ${CMAKE_${CPACK_VISUAL_STUDIO_PRODUCT_NAME}_REDISTRIBUTABLE} NAME )
endif()

# On windows set default install directory appropriately for 32 and 64 bit
# installers if not already set
if(WIN32 AND NOT CPACK_NSIS_INSTALL_ROOT)
  if(CMAKE_CL_64)
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  else()
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
  endif()
endif()

# By default, do not warn when built on machines using only VS Express
if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
endif()

# include required mfc libraries
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "CAS Navigation System")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CAS Navigation System is a part of CAS group")
set(CPACK_PACKAGE_VENDOR "CAS")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${MITK_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${MITK_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${MITK_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${MITK_VERSION_MINOR}")

# tell cpack to strip all debug symbols from all files
set(CPACK_STRIP_FILES ON)

# append revision number if available
if(MITK_REVISION_ID AND MITK_VERSION_PATCH STREQUAL "99")
  if(MITK_WC_TYPE STREQUAL "git")
    set(git_hash ${MITK_REVISION_ID})
    string(LENGTH "${git_hash}" hash_length)
    if(hash_length GREATER 6)
      string(SUBSTRING ${git_hash} 0 6 git_hash)
    endif()
    set(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}_r${git_hash}")
  else()
    set(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}_r${MITK_REVISION_ID}")
  endif()
else()
  set(CPACK_PACKAGE_VERSION_PATCH "${MITK_VERSION_PATCH}")
endif()

# set version
set(CPACK_PACKAGE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# determine possible system specific extension
set(CPACK_PACKAGE_ARCH "unkown-architecture")

if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  if(CMAKE_CL_64)
    set(CPACK_PACKAGE_ARCH "win64")
  elseif(MINGW)
    set(CPACK_PACKAGE_ARCH "mingw32")
  elseif(WIN32)
    set(CPACK_PACKAGE_ARCH "win32")
  endif()
endif(${CMAKE_SYSTEM_NAME} MATCHES Windows)

if(${CMAKE_SYSTEM_NAME} MATCHES Linux)
  if(${CMAKE_SYSTEM_PROCESSOR} MATCHES i686)
    set(CPACK_PACKAGE_ARCH "linux32")
  elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64)
    if(${CMAKE_CXX_FLAGS} MATCHES " -m32 ")
      set(CPACK_PACKAGE_ARCH "linux32")
    else()
      set(CPACK_PACKAGE_ARCH "linux64")
    endif(${CMAKE_CXX_FLAGS} MATCHES " -m32 ")
  else()
    set(CPACK_PACKAGE_ARCH "linux")
  endif()
endif(${CMAKE_SYSTEM_NAME} MATCHES Linux)

if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
  set(CPACK_PACKAGE_ARCH "mac64")
endif(${CMAKE_SYSTEM_NAME} MATCHES Darwin)

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCH}")



