cmake_minimum_required(VERSION 3.5)
project(mqtt_dsr_agent)

# Default to C++20
if(NOT CMAKE_CXX_STANDARD)
  if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(CMAKE_CXX_STANDARD 20)
  else()
    message(FATAL_ERROR "cxx_std_20 could not be found.")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wdeprecated -fPIC -Wshadow -Wnull-dereference)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>")
endif()

find_package(Eigen3 3.3 REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL)
find_package(fastrtps REQUIRED)

include_directories(
  include
  ${QT_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
  ${fastrtps_INCLUDE_DIR}
)

# Set QT libraries and DSR libraries
set(QT_LIBRARIES Qt5::Widgets Qt5::OpenGL Qt5::Core)
set(DSR_LIBRARIES dsr_api dsr_core dsr_gui fastcdr fastrtps)

# Set the names
set(library_name ${PROJECT_NAME}_core)
set(executable_name ${PROJECT_NAME})

# Set  dependencies
set(dependencies
  ${QT_LIBRARIES}
  ${DSR_LIBRARIES}
  Eigen3::Eigen
)

# Set the headers
set(headers
  include/${PROJECT_NAME}/mqtt_agent.hpp
)

# # Qt Moc
qt5_wrap_cpp(qt_moc ${headers}
  OPTIONS --no-notes # Don't display a note for the headers which don't produce a moc_*.cpp
)

# Add library
add_library(${library_name} SHARED src/mqtt_agent.cpp)
target_link_libraries(${library_name} ${dependencies})
target_sources(${library_name} PRIVATE ${qt_moc})

# Add executable
add_executable(${executable_name} src/main.cpp)
target_link_libraries(${executable_name} ${library_name})

# Install
install(TARGETS ${library_name} #${executable_name}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
  FILES_MATCHING PATTERN "*.hpp"
)