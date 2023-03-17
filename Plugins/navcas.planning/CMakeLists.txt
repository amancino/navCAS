project(navcas_planning)

mitk_create_plugin(
  EXPORT_DIRECTIVE PLANNING_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt MitkGizmo MitkSegmentation GraphicsLib NodesManager Interactors CASWidgets
  PACKAGE_DEPENDS Qt5|Sql
)
