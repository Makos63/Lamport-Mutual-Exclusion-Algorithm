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

struct Line {
    int sleepTime;
    int fileSize;
};

struct queueLine{
    int clock;
    std::string process;
    std::string method;
};

class Datasource {
public:
    Datasource();

    Datasource(std::string destIPn, std::string idn, std::string topicn);

    std::string getCurrentTimestamp();

    void readcsv(const std::string &file);

    void run();

    ~Datasource();

private:

    std::vector<Line *> *csvArgs;
    std::string destIP;

    int clock;
    std::queue<queueLine*> *queue;
public:


    std::vector<Line *> *getCsvArgs() const;

    const std::string &getDestIp() const;

    const std::string &getId() const;

    const std::string &getMqttName() const;

    const std::string &getTopic() const;

    void requestToEnter();
    void allowToEnter();
    void release();
    bool allowedToEnter();
    void receive();

private:
    std::string id;
    std::string mqttName;
    std::string topic;

};


#endif //VSEXAM_DATASOURCE_H
