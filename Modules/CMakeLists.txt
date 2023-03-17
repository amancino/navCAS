set(module_dirs
  CASLib
	GraphicsLib
	Interactors
	NodesManager
	CASProject
	CASMapper
  IOUtil
  CASWidgets
  Algorithms
)

foreach(module_dir ${module_dirs})
  add_subdirectory(${module_dir})
endforeach()
