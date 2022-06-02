#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  OSM Data Model Tools - src
#
#-----------------------------------------------------------------------------

add_executable(odmt-characters characters.cpp)
target_link_libraries(odmt-characters ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-characters DESTINATION bin)

add_executable(odmt-duplicate-segments duplicate-segments.cpp)
target_link_libraries(odmt-duplicate-segments ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-duplicate-segments DESTINATION bin)

add_executable(odmt-limits limits.cpp)
target_link_libraries(odmt-limits ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-limits DESTINATION bin)

add_executable(odmt-line-or-polygon line-or-polygon.cpp filter.cpp)
target_link_libraries(odmt-line-or-polygon ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-line-or-polygon DESTINATION bin)

add_executable(odmt-mark-topo-nodes mark-topo-nodes.cpp)
target_link_libraries(odmt-mark-topo-nodes ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-mark-topo-nodes DESTINATION bin)

add_executable(odmt-remove-tags remove-tags.cpp filter.cpp)
target_link_libraries(odmt-remove-tags ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-remove-tags DESTINATION bin)

add_executable(odmt-tag-stats tag-stats.cpp)
target_link_libraries(odmt-tag-stats ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-tag-stats DESTINATION bin)

add_executable(odmt-way-nodes way-nodes.cpp)
target_link_libraries(odmt-way-nodes ${OSMIUM_IO_LIBRARIES})
install(TARGETS odmt-way-nodes DESTINATION bin)

