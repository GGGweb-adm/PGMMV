cmake_minimum_required(VERSION 3.6)

set(lib_name MP3Decoder)
set(target_name ext_${lib_name})

project(${lib_name})

include(../../cmake/CocosExternalConfig.cmake)

add_library(${target_name} STATIC IMPORTED GLOBAL)

set_target_properties(${target_name} PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(${target_name} PROPERTIES
  CC_DEPEND_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/libmpg123.dll"
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/libmpg123.lib"
)
