#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include "src/ORAM.hpp"
#include "src/ORAM.cpp"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "protos/bomap.grpc.pb.h"
#else
#include "bomap.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// ABSL_FLAG(uint16_t, port, 1123, "Server port for the service");

bool is_find = false;
std::chrono::duration<double> server_time(0);

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public bomap::Service {
  Status init_dummy(ServerContext* context, const BucketMessage* request, 
      google::protobuf::Empty* reply) override {
        // level = request->level();
    std::string temp = request->buffer();
    // block temp(request->buffer().begin(), request->buffer().end());
    int len = temp.length();
    switch (request->oram_index())
    {
      case 0:
      {
        // std::cout << "begin Write [leafnode] " << len << " bits to: " << request->position() << std::endl;
        oram3->InitBucket(0, request->position(), temp);
        break;
      }
      case 1:
      {
        // std::cout << "begin Write [midNode2] " << len << " bits to: " << request->position() << std::endl;
        oram2->InitBucket(1, request->position(), temp);
        break;
      }
      default:
      {
        if(request->oram_index() == level - 1)
        {
          mc->Insert(level - 1, 0, request->buffer());
          break;
        }
        // std::cout << "begin Write [midNode1] " << len << " bits to: " << request->position() << std::endl;
        oram1[request->oram_index() - 2].InitBucket(request->oram_index(), request->position(), temp);
        break;
      }
    }
    
    return Status::OK;
        
  }
  Status read_bucket(ServerContext* context, const BucketReadMessage* request,
                  BucketReadResponse* reply) override {
    std::string result;
    auto begin = std::chrono::high_resolution_clock::now();
    switch (request->oram_index())
    {
      case 0:
      {
        if(recv_count3 % 2 == 0)
        {
          oram3->start();
        }
        oram3->FetchPath(0, result, request->path());
        int len = result.length();
        // std::cout << " read [leafNode] " << request->path() << " bits: " << len << std::endl;
        recv_count3++;      
        break;
      }
      case 1:
      {
        if(recv_count2 % 2 == 0)
        {
          oram2->start();
        }
        
        oram2->FetchPath(1, result, request->path());
        int len = result.length();
        // std::cout << " read [midNode2] " << request->path() << " bits: " << len << std::endl;
        recv_count2++;
        break;
      }
      default:
      {
        if(request->oram_index() == level - 1)
        {
          result = mc->Query(level - 1, 0);
          break;
        }
        int32_t index = request->oram_index()-2;
        if(recv_count1[index] % 2 == 0)
        {
          oram1[index].start();
        }
        oram1[index].FetchPath(request->oram_index(), result, request->path());
        int len = result.length();
        // std::cout << " read [midNode1] " << request->path() << " -- " << index << " bits: " << len << std::endl;
        recv_count1[index]++;
        break;
      }
    }
    // string temp(result.begin(), result.end());
    reply->set_buffer(result);
    auto end = std::chrono::high_resolution_clock::now();
    // if(!is_find) {
        server_time += (end - begin)/insert_pairs;
    // }
    return Status::OK;
  }
  Status write_bucket(ServerContext* context, const BucketWriteMessage* request, 
		  google::protobuf::Empty* reply) override {
    auto begin = std::chrono::high_resolution_clock::now();
    int num = request->num();
    
    // std::string temp = request->buffer();
    // block temp(request->buffer().begin(), request->buffer().end());
    // int len = temp.length();
    switch (request->oram_index())
    {
      case 0:
      {
        // std::cout << "begin Write [leafnode] " << len << " bits to: " << request->position() << std::endl;
        for(int i = 0; i < num; i++)
        {
          int p = request->position(i);
          std::string temp = request->buffer(i);
          oram3->WriteBucket(0, p, temp);
        }
        
        break;
      }
      case 1:
      {
        // std::cout << "begin Write [midNode2] " << len << " bits to: " << request->position() << std::endl;
        for(int i = 0; i < num; i++)
        {
          int p = request->position(i);
          std::string temp = request->buffer(i);
          oram2->WriteBucket(1, p, temp);
        }
        break;
      }
      default:
      {
        if(request->oram_index() == level - 1)
        {
          mc->Update(level - 1, 0, request->buffer(0));
          break;
        }
        // std::cout << "begin Write [midNode1] " << len << " bits to: " << request->position() << std::endl;
        for(int i = 0; i < num; i++)
        {
          int p = request->position(i);
          std::string temp = request->buffer(i);
          oram1[request->oram_index() - 2].WriteBucket(request->oram_index(), p, temp);
        }
        
        break;
      }
    }
    auto end = std::chrono::high_resolution_clock::now();
    // if(!is_find) {
        server_time += (end - begin)/insert_pairs;
    // }
    return Status::OK;
  }
  Status end_signal(ServerContext* context, const endMessage* request, 
		  google::protobuf::Empty* reply) override {
    if(request->end() == "search")
    {
      is_find = true;
    }
    if(request->end() == "exit")
    {
      ofstream fout;
	    fout.open("log_server.csv", ios::app);
      fout << N_pairs << "," << L << "," << server_time.count() << std::endl;
    }
    // std::cout << prefix << std::endl;
    return Status::OK;
  }
  Status Setup(ServerContext* context, const SetupRequest* request,
      SetupResponse* reply) override {
        level = request->level();
        is_find = request->is_find();
        if(init == false) {
          mc = new Connector("mongodb://localhost:27017", level, std::to_string(int(log2(N_pairs))), is_find);
          init = true;
        }
        switch (request->oramindex())
        {
          case 0:
          {
            // bytes<Key> key3{2};
            oram3.reset(new ORAM<leafNode>(request->maxsize(), 0, mc, is_find));
            break;
          }
          case 1:
          {
            // bytes<Key> key2{1};
            oram2.reset(new ORAM<midNode2>(request->maxsize(), 1, mc, is_find));
            break;
          }
          default:
          {
            recv_count1.push_back(0);
            
            // bytes<Key> tempkey{(uint8_t)(request->oramindex() - 2)};
            ORAM<midNode1> oram_temp(request->maxsize(), request->oramindex(), mc, is_find);
            oram1.push_back(oram_temp);
            
          }
        }
        return Status::OK;
      }


private:
  Connector* mc;
  std::vector<ORAM<midNode1>> oram1;
  // ORAM<midNode2> oram2;
  std::unique_ptr<ORAM<leafNode>> oram3;
  std::unique_ptr<ORAM<midNode2>> oram2;

  vector<int> recv_count1;
  int recv_count2 = 0;
  int recv_count3 = 0;
  int level;
  bool init = false;
};

void RunServer() {
  std::string server_address = "0.0.0.0:1123";
  GreeterServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  // absl::ParseCommandLine(argc, argv);
  RunServer(/*absl::GetFlag(FLAGS_port)*/);
  return 0;
}
