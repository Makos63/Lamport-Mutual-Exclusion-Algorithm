//
// Created by Maklowski on 2021-03-08.
//

#include "Datasource.h"
#include <unistd.h>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>


void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg;
void on_connect(struct mosquitto *mosq, void *obj, int rc);
void subscriber(std::string brokerIP);

int main(int argc, char *argv[]) {
    std::string destIP, file, id;


    srand(time(NULL) + 1000 * getpid());

    destIP = argv[1];
    file = argv[2];
    id = argv[3];
    //std::cout << file << std::endl;


    Datasource *myDatasource = new Datasource(destIP, id);
    myDatasource->readcsv(file);
    myDatasource->run();

}

std::string Datasource::getCurrentTimestamp() {
    std::array<char, 64> buffer = {0};
    time_t rawtime;
    time(&rawtime);
    const auto timeinfo = localtime(&rawtime);
    strftime(buffer.data(), sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
    std::string timeStr(buffer.data());
    return timeStr;
}

void Datasource::readcsv(const std::string &file) {

    std::ifstream myFile(file);

    std::string tmp, tmp1, tmp2;
    if (myFile.good()) {
        while (std::getline(myFile, tmp, '\n')) {
            std::istringstream iss(tmp);
            getline(iss, tmp1, ',');
            getline(iss, tmp2);

            Line *l = new Line();
            l->sleepTime = std::stoi(tmp1);
            l->fileSize = std::stoi(tmp2);
            csvArgs->push_back(l);

        }
        myFile.close();
    } else {
        std::cout << "could not read the csv file" << std::endl;
    }
    for (auto a: *csvArgs) {
        std::cout << a->sleepTime << " " << a->fileSize << std::endl;
    }
}

Datasource::Datasource(std::string destIPn, std::string idn) {
    csvArgs = new std::vector<Line *>;
    destIP = std::move(destIPn);
    id = std::move(idn);
}

void Datasource::run() {
    std::string mqttName = "Datasource-" + id;
    std::string topic = "Lamports";
    int rc;
    struct mosquitto *mosq;

    while (true) {
        try {


            std::cout << "starting: " << mqttName << std::endl;

            mosquitto_lib_init();
            mosq = mosquitto_new(mqttName.c_str(), true, NULL);

            rc = mosquitto_connect(mosq, destIP.c_str(), 1883, 60);
            //check if connection is successful. if so = 0, if not !=0
            if (rc != 0) {
                mosquitto_destroy(mosq);
                throw "Client could not connect to broker!";
            }

            for (int i = 0; i < csvArgs->size(); ++i) {
                Line *currentLine = csvArgs->at(i);

                sleep(currentLine->sleepTime);

                std::string message;
                for (int i = 0; i < currentLine->fileSize; ++i) {
                    message += "a";
                }

                mosquitto_publish(mosq, NULL, topic.c_str(), message.length(), message.c_str(), 0, false);

            }
        } catch (char const *c) {
            std::cout << "failed with: " << std::endl;
            std::cout << c << std::endl;
            std::cout << "restarting datasource" << std::endl;
            mosq = mosquitto_new(mqttName.c_str(), true, NULL);
            sleep(5);
        }


    }
}

Datasource::Datasource() {

}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);
    //std::cout << "Message" << std::endl;
    //get topic and the payload of the mosquitto message
    //std::cout << "Topic: " << msg->topic << std::endl;
    //std::cout << "Message: " << (char *) msg->payload << std::endl;


    std::string tmp((char *) msg->payload);
    std::stringstream dataS(tmp);
    std::string type, data, senIP, timestamp;

    //split the payload (string) into the values with delimiter
    std::getline(dataS, type, '|');
    std::getline(dataS, data, '|');
    std::getline(dataS, senIP, '|');
    std::getline(dataS, timestamp);
    sensorDataMap.find(type)->second->addData(data, timestamp, senIP);

    //saves timestamp for sensorStatus calculation
    sensorTimes->find(type)->second=timestamp;

    //data transfer to provider. if it fails because provider is down -> test next provider.
    //try for so long until data is send successfully
    bool transferComplete = false;
    std::cout << "transferring data to provider" << std::endl;
    while (!transferComplete) {
        std::cout <<"trying..."<< std::endl;
        try {
            if (!myCentral->TransferDataRPC(type, data, timestamp,centralID)) {
                throw "gRPC failed while connecting to provider";
            } else {
                transferComplete = true;
                std::cout <<"Transfer succeed"<<std::endl;
            }
        } catch (char const *c) {
            std::cout << c << std::endl;
            std::cout << "Trying next provider.. " << std::endl;
            std::cout << "using following provider: " << myCentral->getNextProvider() << std::endl;
        }
    }
}

//test for connection and connect, if successful
void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    std::cout << "ID: " << *(int *) obj << std::endl;
    if (rc != 0) {
        std::cout << "Conenction to broker failed" << std::endl;
        exit(-1);
    }
    //clientobj, messageid, topic, quality for service level
    mosquitto_subscribe(mosq, NULL, centralTopic.c_str(), 0);
}


void subscriber(std::string brokerIP) {
    int rc, id = std::stoi(centralID);

    mosquitto_lib_init();
    //client obj struct
    struct mosquitto *mosq;
    //create object
    //clientname, clean session, subscriber id
    std::string t = "sub-" + centralID;
    mosq = mosquitto_new(t.c_str(), true, &id);

    //void (Central::*func)(struct mosquitto *, void *, int );
    //func = &Central::on_connect;

    //callback if client is connected to broker
    mosquitto_connect_callback_set(mosq, on_connect);
    //callback if client recieves a massage
    mosquitto_message_callback_set(mosq, on_message);

    //connect to broker
    rc = mosquitto_connect(mosq, brokerIP.c_str(), 1883, 10);
    if (rc) {
        throw "Could not connect to Broker";
    }


    //loop for listening
    mosquitto_loop_start(mosq);
    printf("Press Enter to quit...\n");
    //quit loop with force or not
    mosquitto_loop_stop(mosq, false);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

}
