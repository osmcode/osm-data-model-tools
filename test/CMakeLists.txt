#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  OSM Data Model Tools - Tests
#
#-----------------------------------------------------------------------------

add_test(NAME line-or-polygon
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/line-or-polygon.sh ${CMAKE_SOURCE_DIR})

