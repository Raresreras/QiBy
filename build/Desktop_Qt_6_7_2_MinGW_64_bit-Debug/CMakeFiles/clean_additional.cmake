# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\QiBy_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\QiBy_autogen.dir\\ParseCache.txt"
  "QiBy_autogen"
  )
endif()
