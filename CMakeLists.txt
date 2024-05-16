
cmake_minimum_required(VERSION 3.8)

project(Bomap C CXX)

include(cmake/common.cmake)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb --std=c++17")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall --std=c++17")

# Proto file
get_filename_component(hw_proto "protos/bomap.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/bomap.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/bomap.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/bomap.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/bomap.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

file(GLOB_RECURSE ALL_LIB_SRC
  src/*.cpp
)

find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs}
  ${ALL_LIB_SRC})
target_link_libraries(hw_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  mongo::bsoncxx_shared
  mongo::mongocxx_shared)

# Add all src files
# set(src_file "src/ORAM.cpp" "src/node.cpp" "src/RAMStore.cpp" "src/AES.cpp" "src/Bid.cpp")
# add_library(src_lib ${src_file})
# target_link_libraries(src_lib
#   ${_REFLECTION}
#   ${_GRPC_GRPCPP}
#   ${_PROTOBUF_LIBPROTOBUF})

# add_subdirectory(src)
# add_library(my_lib ${ALL_LIB_SRC})
# target_link_libraries(my_lib
#   hw_grpc_proto
#   ${_REFLECTION}
#   ${_GRPC_GRPCPP}
#   ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
foreach(_target
   test_bomap test_server)
  # greeter_callback_client greeter_callback_server
  # greeter_async_client greeter_async_client2 greeter_async_server)
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
    hw_grpc_proto
    absl::flags
    absl::flags_parse
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

# cmake_minimum_required(VERSION 3.8)

# project(Test C CXX)

# add_executable(test "test.cpp" "src/ORAM.cpp" "src/node.cpp" "src/RAMStore.cpp" "src/AES.cpp" "src/Bid.cpp")

# target_link_libraries(test )