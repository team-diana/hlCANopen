include_directories(${CMAKE_CURRENT_SOURCE_DIR} cansim/include include/)
include_directories(include/hlcanopen/)


#add_library(cansim SHARED
              #${CMAKE_CURRENT_SOURCE_DIR}/cansim/src/bus.cpp)
#              )

find_package( Threads )

find_package (Boost COMPONENTS unit_test_framework system REQUIRED)
include_directories (${TEST_SOURCE_DIR}/src
                     ${Boost_INCLUDE_DIRS}
                        dlib
                     )
add_definitions (-DBOOST_TEST_DYN_LINK)

add_library(TestUtils test_utils.cpp)

function(add_canopen_test TestName files)
  add_executable (${TestName} ${files})
  target_link_libraries (${TestName}
                        hlcanopen
                        TestUtils
                        ${Boost_LIBRARIES}
                        ${CMAKE_THREAD_LIBS_INIT} 
                        )
endfunction(add_canopen_test)

add_canopen_test(Cansim_test "cansim_test.cpp")
add_canopen_test(SdoClientRequest_test "sdo_client_request_test.cpp")
add_canopen_test(SdoServerRequest_test "sdo_server_request_test.cpp")
add_canopen_test(SdoClientNodeManager_test "sdo_client_node_manager_test.cpp")
add_canopen_test(SdoCanOpenManager_test "sdo_can_open_manager_test.cpp")
add_canopen_test(PdoConfiguration_test "pdo_configuration_test.cpp")
add_canopen_test(PdoClient_test "pdo_client_test.cpp")
add_canopen_test(SdoCanOpenManagerTimeout_test "sdo_can_open_manager_timeout_test.cpp")
