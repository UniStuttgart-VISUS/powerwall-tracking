###############################################################################
# TEST
###############################################################################

project(test)
message(STATUS "Configure '${PROJECT_NAME}'")

# TARGET DEFINITION
add_executable(${PROJECT_NAME} 
    ${TEST_HEADERS} 
    ${TEST_SOURCES}
)

target_sources(${PROJECT_NAME} PRIVATE
    ### SOURCE ###
    "src/test.cpp"
)
include_directories("${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/include")

target_include_directories(glm INTERFACE glm)
target_include_directories(vrpn INTERFACE vrpn)
target_include_directories(natnet INTERFACE ${NATNET_INCLUDE_DIR})
target_include_directories(tracking INTERFACE "${CMAKE_INSTALL_PREFIX}/tracking/include")

target_link_libraries(${PROJECT_NAME} PUBLIC tracking natnet vrpn glm)
     
# INSTALL TARGET
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

# Grouping in Visual Studio
source_group("Header Files" FILES ${TEST_HEADERS})
source_group("Source Files" FILES ${TEST_SOURCES})
