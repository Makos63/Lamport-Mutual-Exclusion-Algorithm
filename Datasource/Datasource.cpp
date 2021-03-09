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
#include <thread>


void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

void on_connect(struct mosquitto *mosq, void *obj, int rc);

void subscriber(std::string brokerIP);

Datasource *myDatasource;

int main(int argc, char *argv[]) {
    std::string destIP, file, id;


    srand(time(NULL) + 1000 * getpid());

    destIP = argv[1];
    file = argv[2];
    id = argv[3];
    //std::cout << file << std::endl;

    std::string topic = "Lamports";

    myDatasource = new Datasource(destIP, id, topic);
    myDatasource->readcsv(file);
//    myDatasource->run();
    std::thread pubThread(&Datasource::run, myDatasource);

    subscriber(myDatasource->getDestIp());

    pubThread.join();
}

std::string Datasource::getCurrentTimestamp() {
    std::time_t result = std::time(nullptr);
    //std::cout << std::asctime(std::localtime(&result)) << result << " seconds since the Epoch\n";
    return reinterpret_cast<const char *>(result);
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

Datasource::Datasource(std::string destIPn, std::string idn, std::string topicn) {
    csvArgs = new std::vector<Line *>;
    destIP = std::move(destIPn);
    id = std::move(idn);
    topic = std::move(topicn);
    mqttName = "Datasource-" + id;
    queue = new std::queue<queueLine *>;
    clock = 0;
}

void Datasource::run() {

    int rc;
    struct mosquitto *mosq;

    //subscriber(destIP);











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
        std::string message = "";
        for (int i = 0; i < csvArgs->size(); ++i) {
            Line *currentLine = csvArgs->at(i);

            sleep(currentLine->sleepTime);


            requestToEnter();
            message = std::to_string(queue->front()->clock) + queue->front()->process + queue->front()->method;
            mosquitto_publish(mosq, NULL, topic.c_str(), message.length(), message.c_str(), 0, false);

            if (allowedToEnter()) {
                /*send to datastore
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
}*/
            }
            release();
        }
    } catch (char const *c) {
        std::cout << "failed with: " << std::endl;
        std::cout << c << std::endl;
    }

}

Datasource::Datasource() {

}

std::vector<Line *> *Datasource::getCsvArgs() const {
    return csvArgs;
}

const std::string &Datasource::getDestIp() const {
    return destIP;
}

const std::string &Datasource::getId() const {
    return id;
}

const std::string &Datasource::getMqttName() const {
    return mqttName;
}

const std::string &Datasource::getTopic() const {
    return topic;
}

Datasource::~Datasource() {

}

void Datasource::requestToEnter() {
    ++clock;
    queueLine *ql = new queueLine();
    ql->clock = clock;
    ql->method = "Enter";
    ql->process = id;
    queue->push(ql);
}

void Datasource::allowToEnter() {

}

void Datasource::release() {

}

bool Datasource::allowedToEnter() {
    return false;
}

void Datasource::receive() {

}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);


}

//test for connection and connect, if successful
void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    std::cout << "ID: " << *(int *) obj << std::endl;
    if (rc != 0) {
        std::cout << "Conenction to broker failed" << std::endl;
        exit(-1);
    }
    //clientobj, messageid, topic, quality for service level
    mosquitto_subscribe(mosq, NULL, myDatasource->getTopic().c_str(), 0);
}


void subscriber(std::string brokerIP) {
    int rc, id = std::stoi(myDatasource->getId());

    mosquitto_lib_init();
    //client obj struct
    struct mosquitto *mosq;
    //create object
    //clientname, clean session, subscriber id
    std::string t = myDatasource->getMqttName();
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
