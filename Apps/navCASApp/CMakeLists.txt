project(navCASApp)

set(_app_options)
if(${MY_PROJECT_NAME}_SHOW_CONSOLE_WINDOW)
  list(APPEND _app_options SHOW_CONSOLE)
endif()

# Plug-ins listed below will not be
#  - added as a build-time dependency to the executable
#  - listed in the provisioning file for the executable
#  - installed if they are external plug-ins

set(_exclude_plugins
  org.blueberry.ui.qt.help
  org.blueberry.uitest
  org.mitk.planarfigure
  org.mitk.gui.qt.volumevisualization
  org.mitk.gui.qt.ultrasound
  org.mitk.gui.qt.ugvisualization
  org.mitk.gui.qt.tofutil
  org.mitk.gui.qt.toftutorial
  org.mitk.gui.qt.python.console
  org.mitk.gui.qt.pointsetinteraction
  org.mitk.gui.qt.moviemaker
  org.mitk.gui.qt.meshdecimation
  org.mitk.gui.qt.materialeditor
  org.mitk.gui.qt.imagecropper

  #org.mitk.gui.qt.extapplication
  org.mitk.gui.qt.examplesopencv
  org.mitk.gui.qt.examples
  org.mitk.gui.qt.dtiatlasapp
  org.mitk.gui.qt.diffusionimagingapp
  org.mitk.gui.qt.diffusionimaging
  org.mitk.gui.qt.basicimageprocessing
  org.mitk.gui.qt.coreapplication
  org.mitk.diffusionimaging
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.dicom
  org.mitk.gui.qt.flow.segmentation
  org.mitk.gui.qt.registration
  org.mitk.gui.qt.segmentation
  org.mitk.gui.qt.cmdlinemodules
  org.mitk.gui.qt.datamanagerlight
  
  #org.mitk.gui.qt.properties
  org.mitk.views.propertylistview
  org.blueberry.ui.qt.log
  org.blueberry.views.logview

  org.mitk.gui.qt.xnat
  org.mitk.gui.qt.eventrecorder
  org.mitk.simulation
  org.mitk.gui.qt.aicpregistration
  org.mitk.gui.qt.dosevisualization
  org.mitk.gui.qt.overlaymanager
  org.mitk.gui.qt.pointsetinteractionmultispectrum
  org.blueberry.ui.qt.objectinspector
  org.mitk.gui.qt.multilabelsegmentation
  org.mitk.gui.qt.tubegraph
  org.mitk.gui.qt.geometrytools
  org.mitk.gui.qt.dicominspector
  #org.mitk.gui.qt.stdmultiwidgeteditor
  org.mitk.gui.qt.renderwindowmanager
  org.mitk.gui.qt.classificationsegmentation
  org.mitk.gui.qt.cest
  org.mitk.gui.qt.remeshing
  org.mitk.gui.qt.matchpoint.algorithm.batch
  org.mitk.gui.qt.matchpoint.algorithm.browser
  org.mitk.gui.qt.matchpoint.algorithm.control
  org.mitk.gui.qt.matchpoint.evaluator
  org.mitk.gui.qt.matchpoint.framereg
  org.mitk.gui.qt.matchpoint.manipulator
  org.mitk.gui.qt.matchpoint.mapper
  org.mitk.gui.qt.matchpoint.visualizer
  org.mitk.gui.qt.openigtlink

  org.mitk.gui.qt.photoacoustics.imageprocessing
  org.mitk.gui.qt.photoacoustics.pausviewer
  org.mitk.gui.qt.photoacoustics.simulation
  org.mitk.gui.qt.photoacoustics.pausmotioncompensation
  org.mitk.gui.qt.photoacoustics.spectralunmixing

  org.mitk.gui.qt.python

  org.mitk.gui.qt.deformableclippingplane
  org.mitk.gui.qt.igt.app.echotrack
  org.mitk.gui.qt.igt.app.hummelprotocolmeasurements
  org.mitk.gui.qt.igtexamples
  org.mitk.gui.qt.igttracking
  org.mitk.gui.qt.igt.app.ultrasoundtrackingnavigation
  #org.mitk.gui.qt.measurementtoolbox
  org.mitk.gui.qt.diffusionimaging
  org.mitk.gui.qt.diffusionimaging.connectomics
  org.mitk.gui.qt.diffusionimaging.denoising
  org.mitk.gui.qt.diffusionimaging.fiberfox
  org.mitk.gui.qt.diffusionimaging.fiberprocessing
  org.mitk.gui.qt.diffusionimaging.ivim
  org.mitk.gui.qt.diffusionimaging.odfpeaks
  org.mitk.gui.qt.diffusionimaging.partialvolume
  org.mitk.gui.qt.diffusionimaging.preprocessing
  org.mitk.gui.qt.diffusionimaging.reconstruction
  org.mitk.gui.qt.diffusionimaging.registration
  org.mitk.gui.qt.diffusionimaging.tbss
  org.mitk.gui.qt.diffusionimaging.tractography
  org.mitk.gui.qt.diffusionimaging.python
  org.mitk.gui.qt.preprocessing.resampling
  org.mitk.gui.qt.datastorageviewertest
  org.mitk.gui.qt.viewnavigator
  

  org.mitk.gui.qt.fit.demo
  org.mitk.gui.qt.fit.inspector
  org.mitk.gui.qt.fit.genericfitting
  org.mitk.gui.qt.pharmacokinetics.mri
  org.mitk.gui.qt.pharmacokinetics.pet
  org.mitk.gui.qt.pharmacokinetics.simulation
  org.mitk.gui.qt.pharmacokinetics.curvedescriptor
  org.mitk.gui.qt.pharmacokinetics.concentration.mri

  org.mitk.gui.qt.radiomics
  org.mitk.gui.qt.semanticrelations
  org.mitk.gui.qt.chartExample
  #org.mitk.gui.qt.datamanager
  org.mitk.gui.qt.mxnmultiwidgeteditor
)

mitkFunctionCreateBlueBerryApplication(
  NAME ${MY_APP_NAME}
  DESCRIPTION "MITK - ${MY_APP_NAME} Application"
  EXCLUDE_PLUGINS ${_exclude_plugins}
  ${_app_options}
)
mitk_use_modules(TARGET ${MY_APP_NAME}
  MODULES MitkAppUtil
  PACKAGES Qt5|Widgets
)
