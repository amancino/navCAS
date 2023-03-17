MITK_CREATE_MODULE_TESTS()

if(TARGET ${TESTDRIVER})
  mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES Qt5|Core+Sql VTK)

  # Copy test files
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/calibration1.sqlite
		./data/calibration1.sqlite COPYONLY)
endif()


