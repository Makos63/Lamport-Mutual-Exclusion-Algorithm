//
// Created by Maklowski on 2021-03-10.
//

#ifndef DATASOURCE_GRPCCLIENT_H
#define DATASOURCE_GRPCCLIENT_H

#include <string>

#include <grpcpp/grpcpp.h>
#include "dataproto.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using dataproto::Resource;
using dataproto::ResourceRequest;
using dataproto::ResourcesRequest;
using dataproto::Reply;
using dataproto::HealthCheckRequest;


class GrpcClient {
public:
    GrpcClient(std::shared_ptr<Channel> channel) : stub_(dataproto::Service1::NewStub(channel)){};

    Reply storeData(std::string id, std::string name,
                     std::string messSize,std::string messDesc) {
        Resource ResourceData;

        ResourceData.set_id(std::stoi(id));
        ResourceData.set_name(name);
        ResourceData.set_size(std::stoi(messSize));
        ResourceData.set_description(messDesc);


        Reply result;

        ClientContext context;

        Status status = stub_->storeData(&context, ResourceData, &result);

        if (status.ok()) {
            return result;
        } else {
            std::cout <<"GRPC: status not okay| status code: "<<status.error_code() <<std::endl;
            std::cout <<"ERROR MESSAGE: "<< status.error_message() <<std::endl;
            std::cout <<"ERROR DETAILS: "<< status.error_details()<<std::endl;
        }
    }


    const std::unique_ptr<dataproto::Service1::Stub> &getStub() const {
        return stub_;
    }

private:
    std::unique_ptr<dataproto::Service1::Stub> stub_;
};


#endif //DATASOURCE_GRPCCLIENT_H
