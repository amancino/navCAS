set(COMMON_FILES
  navAPI.cpp)

IF(WIN32)
  set(CPP_FILES
    ${COMMON_FILES}
    ../helpers_windows.cpp)
ELSE()
  set(CPP_FILES
    ${COMMON_FILES}
    ../../../helpers_linux.cpp)
ENDIF()

set(RESOURCE_FILES
  Interactions/Paint.xml
  Interactions/PaintConfig.xml
)

set(MOC_H_FILES
  include/navAPI.h
)
