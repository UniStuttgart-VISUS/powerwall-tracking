###############################################################################
# EXTERNALS
###############################################################################
message(STATUS "Externals:")
get_cmake_property(_cache_before CACHE_VARIABLES)

# glm
message(STATUS "    Configure 'glm'")
add_library(pwtglm INTERFACE)
target_include_directories(pwtglm INTERFACE pwtglm)

# vrpn
message(STATUS "    Configure 'vrpn'")
# Clear option to omit warning
set(VRPN_USE_LOCAL_HIDAPI "" CACHE STRING "cleared" FORCE)
set(VRPN_INSTALL OFF)
set(VRPN_BUILD_CLIENT_LIBRARY ON)
add_subdirectory(vrpn)
target_include_directories(vrpn INTERFACE vrpn)

# natnet
# Find the NatNet library, the location of the NatNet dll and the location of the include files.
if(TRACKING_USE_NATNET)
    message(STATUS "    Configure 'natnet'")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}) 
    find_package(NATNET REQUIRED)
    if(NATNET_FOUND)
        #message(STATUS "    [debug] NatNet DLL:       ${NATNET_DLL_DIR}")
        #message(STATUS "    [debug] NatNet Include:   ${NATNET_INCLUDE_DIR}")
        #message(STATUS "    [debug] NatNet library:   ${NATNET_GENERIC_LIBRARY}")
        #message(STATUS "    [debug] NetNet Libraries: ${NATNET_LIBRARIES}")    
        add_library(natnet INTERFACE)
        target_include_directories(natnet INTERFACE ${NATNET_INCLUDE_DIR})
        target_link_libraries(natnet INTERFACE ${NATNET_GENERIC_LIBRARY})
    else()
        message(FATAL_ERROR "[${PROJECT_NAME}] Cannot find NatNet SDK")
    endif()
endif()

# Iterate and hide all new names (optional: matching pattern (case-sensitive regex))
get_cmake_property(_cache_after CACHE_VARIABLES)
foreach(_key IN LISTS _cache_after)
  list(FIND _cache_before "${_key}" _found)
  if(_found EQUAL -1) # optional: only newly added keys
    #if(_key MATCHES "^VRPN_") # change regex to your wildcard (e.g. "^FOO|BAR" or ".*_OPTION$")
      # mark advanced (hidden from main list) OR make fully internal
      mark_as_advanced(${_key})
      # set(${_key} "${${_key}}" CACHE INTERNAL "hidden by parent" FORCE)
    #endif()
  endif()
endforeach()