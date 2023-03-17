mitk_create_module(GraphicsLib
  DEPENDS PUBLIC MitkCore MitkAnnotation MitkQtWidgets #MitkMatchPointRegistration
  PACKAGE_DEPENDS VTK|ImagingGeneral ITK Qt5|Core
  #INCLUDE_DIRS "${SQLite3_INCLUDE_DIRS}"
  #ADDITIONAL_LIBS ${SQLite3_LIBRARIES}
  WARNINGS_NO_ERRORS
)
add_subdirectory(test)
