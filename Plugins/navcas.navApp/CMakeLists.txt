PROJECT(navcas_navApp)

mitk_create_plugin(
  EXPORT_DIRECTIVE NAV_APP_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkSceneSerialization MitkAppUtil MitkQtWidgets IOUtil CASMapper #CASProject
)
