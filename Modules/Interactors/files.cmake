file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  PlanningInteractor.cpp
)

set(RESOURCE_FILES
  Interactions/PlanningInteraction.xml
  Interactions/PlanningInteractionConfig.xml
)

set(MOC_H_FILES
  include/PlanningInteractor.h
)
