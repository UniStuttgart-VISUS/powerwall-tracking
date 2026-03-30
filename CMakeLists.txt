###############################################################################
# MAIN
###############################################################################

cmake_minimum_required (VERSION 3.11)
project(powerwall-tracking)

# Set default install path 
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}
      CACHE PATH "default install path" FORCE)
endif()

# Add directory structure for visual studio
if(WIN32)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD TRUE)
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()

# Options
option(TRACKING_USE_NATNET "Use NatNet SDK" ON)
option(TRACKING_TEST_PROGRAM "Add the test program" ON)
set(TRACKING_CONFIG_FILE "${CMAKE_SOURCE_DIR}/tracking/conf/tracking.conf" CACHE PATH "Config file for tracking")

###############################
# EXTERNALS
###############################
add_subdirectory(external)

###############################
# TRACKING
###############################
add_subdirectory(tracking)

###############################
# TEST
###############################
if(TRACKING_TEST_PROGRAM)
    add_subdirectory(test)
endif()
