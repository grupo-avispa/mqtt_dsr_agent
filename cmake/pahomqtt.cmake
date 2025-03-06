if(NOT PahoMqttCpp_FOUND)
    message(STATUS "Downloading paho_mqtt_cpp")
    include(FetchContent)

    set(PAHO_BUILD_SHARED ON CACHE INTERNAL "")
    set(PAHO_BUILD_STATIC ON CACHE INTERNAL "")
    set(PAHO_ENABLE_TESTING OFF CACHE INTERNAL "")
    set(PAHO_BUILD_TESTS OFF CACHE INTERNAL "")
    set(PAHO_WITH_MQTT_C ON CACHE INTERNAL "")

    set(FETCHCONTENT_QUIET ON)
    FetchContent_Declare(paho_mqtt_cpp
        GIT_REPOSITORY    https://github.com/eclipse/paho.mqtt.cpp.git
        GIT_TAG           v1.4.0
        GIT_PROGRESS      TRUE
        GIT_SHALLOW       TRUE
        #SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/paho_mqtt_cpp-src"
        #BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/paho_mqtt_cpp-build"
    )
    FetchContent_MakeAvailable(paho_mqtt_cpp)
    #FetchContent_Populate(paho_mqtt_cpp)
    #add_subdirectory(${paho_mqtt_cpp_SOURCE_DIR} ${paho_mqtt_cpp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
