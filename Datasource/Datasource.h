//
// Created by Maklowski on 2021-03-08.
//

#ifndef VSEXAM_DATASOURCE_H
#define VSEXAM_DATASOURCE_H

#include <string>
#include <map>
#include <vector>
#include <mosquitto.h>

struct Line{
    int sleepTime;
    int fileSize;
};

class Datasource {
public:
    Datasource();

    Datasource(std::string destIP, std::string id);

    std::string getCurrentTimestamp();

    void readcsv(const std::string &file);

    void run();

    void mqttPublischer(std::string data);

private:

    std::vector<Line*> *csvArgs;
    std::string destIP;
    std::string id;

};


#endif //VSEXAM_DATASOURCE_H
