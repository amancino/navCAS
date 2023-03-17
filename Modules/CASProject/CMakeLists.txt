mitk_create_module(CASProject
  INCLUDE_DIRS PRIVATE src
  DEPENDS PUBLIC MitkCore MitkSceneSerialization
  PACKAGE_DEPENDS Poco Qt5|Core

  # force to load module even if no function is ever called
  AUTOLOAD_WITH MitkCore

  #WARNINGS_NO_ERRORS
)


