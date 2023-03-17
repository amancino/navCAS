mitk_create_module(NodesManager
  DEPENDS PUBLIC MitkCore MitkIGT CASLib GraphicsLib
  PACKAGE_DEPENDS VTK ITK Qt5|Core+Sql
  WARNINGS_NO_ERRORS
)

add_subdirectory(test)
