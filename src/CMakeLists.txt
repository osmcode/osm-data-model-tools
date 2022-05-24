#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  OSM Data Model Tools - src
#
#-----------------------------------------------------------------------------

add_executable(duplicate-segments duplicate-segments.cpp)
target_link_libraries(duplicate-segments ${OSMIUM_IO_LIBRARIES})
install(TARGETS duplicate-segments DESTINATION bin)

add_executable(line-or-polygon line-or-polygon.cpp filter.cpp)
target_link_libraries(line-or-polygon ${OSMIUM_IO_LIBRARIES})
install(TARGETS line-or-polygon DESTINATION bin)

add_executable(mark-topo-nodes mark-topo-nodes.cpp)
target_link_libraries(mark-topo-nodes ${OSMIUM_IO_LIBRARIES})
install(TARGETS mark-topo-nodes DESTINATION bin)

add_executable(remove-tags remove-tags.cpp filter.cpp)
target_link_libraries(remove-tags ${OSMIUM_IO_LIBRARIES})
install(TARGETS remove-tags DESTINATION bin)

add_executable(tag-stats tag-stats.cpp)
target_link_libraries(tag-stats ${OSMIUM_IO_LIBRARIES})
install(TARGETS tag-stats DESTINATION bin)

add_executable(way-nodes way-nodes.cpp)
target_link_libraries(way-nodes ${OSMIUM_IO_LIBRARIES})
install(TARGETS way-nodes DESTINATION bin)

