project(navcas_registrationPlanning)

mitk_create_plugin(
  EXPORT_DIRECTIVE REGISTRATION_PLANNING_EXPORTS
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS PRIVATE MitkQtWidgetsExt Interactors NodesManager CASWidgets
)
