cmake_minimum_required(VERSION 2.6)
project(hlcanopen)

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
if(COMPILER_SUPPORTS_CXX1Y)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++1y (C++14) support. Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

SET(BOOST_MIN_VERSION "1.57.0")
find_package(Boost
  ${BOOST_MIN_VERSION}
  REQUIRED
  COMPONENTS system coroutine filesystem
)

include_directories(include/)
include_directories(${Boost_INCLUDE_DIRS})

add_library(hlcanopen SHARED
            src/sdo_error.cpp
            src/types.cpp
            src/can_msg.cpp
            src/can_msg_utils.cpp
            src/logging.cpp
            src/utils.cpp
            src/sdo_entry_converter_visitor.cpp
            src/logging/logging_conf_loader.cpp
            src/executor/unique_thread_executor.cpp
            src/sdo_client.cpp
            src/sdo_client_node_manager.cpp
            src/sdo_data_converter.cpp
            src/can_open_manager.cpp
            src/node_manager.cpp
          )
target_link_libraries(hlcanopen folly atomic pthread glog ${Boost_LIBRARIES})

install(TARGETS hlcanopen
  LIBRARY DESTINATION lib)
install(DIRECTORY include/
  DESTINATION include)
install(FILES hlcanopen-config.cmake DESTINATION lib/cmake/hlcanopen/)

add_subdirectory(test)

enable_testing()

add_test (NAME Cansim COMMAND Cansim_test)
add_test (NAME CanMsg COMMAND CanMsg_test)
add_test (NAME SdoClientRequest COMMAND SdoClientRequest_test)
add_test (NAME SdoServerRequest COMMAND SdoServerRequest_test)
add_test (NAME SdoClientNodeManager COMMAND SdoClientNodeManager_test)
add_test (NAME SdoCanOpenManager COMMAND SdoCanOpenManager_test)
add_test (NAME PdoConfiguration COMMAND PdoConfiguration_test)
add_test (NAME SdoCanOpenManagerTimeout COMMAND SdoCanOpenManagerTimeout_test)

