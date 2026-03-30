###############################################################################
# TRACKING
###############################################################################

project(tracking)
message(STATUS "Configure '${PROJECT_NAME}'")

# TARGET DEFINITION
add_library(${PROJECT_NAME} SHARED ${TRACKING_HEADERS} ${TRACKING_SOURCES})

target_sources(${PROJECT_NAME} PRIVATE
    ### HEADERS ###
    "include/VrpnButtonDevice.h"
    "include/VrpnDevice.h"
    "include/NatNetDevicePool.h"
    "include/Tracker.h"
    "include/TrackingUtilizer.h"
    ### SOURCE ###
    "src/Tracker.cpp"
    "src/TrackingUtilizer.cpp"
    "src/VrpnButtonDevice.cpp"
    "src/NatNetDevicePool.cpp"
)
include_directories("${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/src")

string(TOUPPER ${PROJECT_NAME} EXPORT_NAME)
target_compile_definitions(${PROJECT_NAME} PRIVATE "${EXPORT_NAME}_EXPORTS")

target_link_libraries(${PROJECT_NAME} PUBLIC natnet vrpn glm)

# INSTALLATION
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME} ${PROJECT_NAME} 
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
    LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" 
)
install(FILES ${NATNET_DLL_DIR} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(FILES ${TRACKING_CONFIG_FILE} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

# Grouping in Visual Studio
source_group("Header Files" FILES ${TRACKING_HEADERS})
source_group("Source Files" FILES ${TRACKING_SOURCES})
