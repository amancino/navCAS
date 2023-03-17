project(navcas_navigationPlugins)

mitk_create_plugin(
  EXPORT_DIRECTIVE NAVIGATION_BASE_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt CASLib NodesManager IOUtil CASWidgets Algorithms
  PACKAGE_DEPENDS PUBLIC Qt5|Core+Multimedia+Sql
  WARNINGS_NO_ERRORS
)
