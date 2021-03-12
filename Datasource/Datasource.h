//
// Created by Maklowski on 2021-03-08.
//

#ifndef VSEXAM_DATASOURCE_H
#define VSEXAM_DATASOURCE_H

#include <string>
#include <map>
#include <vector>
#include <mosquitto.h>
#include <queue>
#include <algorithm>
#include <mutex>
#include "grpcClient.h"

struct Line {
    int sleepTime;
    int fileSize;
};

struct queueLine {
    int clock;
    std::string process;
};

std::mutex g_mutex;

class Datasource {
private:
    std::string id;
    std::string mqttName;
    std::string topic;
    std::vector<Line *> *csvArgs;
    std::string destIP;
    int messCounter=0;
    GrpcClient *myClient;

    int clock;
    std::vector<queueLine *> *queue;
    int ackCounter;
    int sourceCount;
public:
    Datasource();

    Datasource(std::string destIPn, std::string idn, std::string topicn,std::string count,std::string grpcDest);

    std::string getCurrentTimestamp();

    void printVector();

    void readcsv(const std::string &file);

    void run();

    ~Datasource();

    std::vector<Line *> *getCsvArgs() const;

    const std::string &getDestIp() const;

    const std::string &getId() const;

    const std::string &getMqttName() const;

    const std::string &getTopic() const;

    void requestToEnter();

    void allowToEnter(std::string requester);

    void release();

    bool allowedToEnter();

    void receive(std::string recMessage);


};


#endif //VSEXAM_DATASOURCE_H
