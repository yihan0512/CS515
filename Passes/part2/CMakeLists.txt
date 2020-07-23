add_llvm_library( submission_pt2 MODULE
  ReachingDefinitionAnalysis.cpp
  ConstPropAnalysis.cpp

  PLUGIN_TOOL
  opt
  )
