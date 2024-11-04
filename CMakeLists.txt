cmake_minimum_required(VERSION 3.5)
project(mqtt_dsr_agent)

include(cmake/pahomqtt.cmake)

# Default to C++20
if(NOT CMAKE_CXX_STANDARD)
  if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(CMAKE_CXX_STANDARD 20)
  else()
    message(FATAL_ERROR "cxx_std_20 could not be found.")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Werror -Wdeprecated -fPIC -Wshadow -Wnull-dereference)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>")
endif()

find_package(Eigen3 3.3 REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(fastrtps REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

include_directories(
  include
  ${QT_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
  ${fastrtps_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/paho_mqtt_cpp-src/include
)

# Set QT libraries and DSR libraries
set(QT_LIBRARIES Qt5::Core)
set(DSR_LIBRARIES dsr_api dsr_core fastcdr fastrtps)
set(PahoMqtt_LIBRARIES pthread crypto ssl paho-mqtt3as paho-mqttpp3)

# Set the names
set(library_name ${PROJECT_NAME}_core)
set(executable_name ${PROJECT_NAME})

# Set  dependencies
set(dependencies PUBLIC
  ${QT_LIBRARIES}
  ${DSR_LIBRARIES}
  Eigen3::Eigen
  ${PahoMqtt_LIBRARIES}
  PRIVATE
  nlohmann_json::nlohmann_json
)

# Set the headers
set(headers
  include/${PROJECT_NAME}/mqtt_agent.hpp
  include/${PROJECT_NAME}/campero_types.hpp
)

# # Qt Moc
qt5_wrap_cpp(qt_moc ${headers}
  OPTIONS --no-notes # Don't display a note for the headers which don't produce a moc_*.cpp
)

# Add library
add_library(${library_name} SHARED src/mqtt_agent.cpp)
target_include_directories(${library_name} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
)
target_link_libraries(${library_name} ${dependencies})
target_sources(${library_name} PRIVATE ${qt_moc})

# Add executable
add_executable(${executable_name} src/main.cpp)
target_link_libraries(${executable_name} ${library_name})

# Install
install(TARGETS ${library_name} ${executable_name}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include/
)