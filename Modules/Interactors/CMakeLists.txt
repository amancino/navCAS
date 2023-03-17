mitk_create_module(Interactors
  INCLUDE_DIRS PRIVATE src
  DEPENDS PUBLIC MitkCore GraphicsLib
  PACKAGE_DEPENDS VTK ITK Qt5|Core
  WARNINGS_NO_ERRORS
)
