if(WIN32)
  mitk_create_module(CASLib
    DEPENDS PUBLIC MitkCore
    PACKAGE_DEPENDS Qt5|Core
    INCLUDE_DIRS
      PRIVATE ${ATRACSYS_SDK_INSTALL_PATH}/samples
    ADDITIONAL_LIBS ${LIBS} 
                    "${ATRACSYS_SDK_INSTALL_PATH}/lib/fusionTrack64.lib"
    WARNINGS_NO_ERRORS
    )
else(WIN32)
  mitk_create_module(CASLib
    DEPENDS PUBLIC MitkCore
    PACKAGE_DEPENDS Qt5|Core
    INCLUDE_DIRS
      PRIVATE ${ATRACSYS_SDK_INSTALL_PATH}/samples
    ADDITIONAL_LIBS ${LIBS}
    WARNINGS_NO_ERRORS
    )
endif(WIN32)
