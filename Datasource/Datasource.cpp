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

void subscriber();

void publish(std::string mess);

Datasource *myDatasource;


int rcSUB;
struct mosquitto *mosqSUB;


int main(int argc, char *argv[]) {
    std::string destIP, file, id, sourceCount, grpcDest;
    std::cout << "lets go!" << std::endl;

    srand(time(NULL) + 1000 * getpid());

    destIP = argv[1];
    file = argv[2];
    id = argv[3];
    sourceCount = argv[4];
    grpcDest = argv[5];

    std::string topic = "t/Lamports";

    myDatasource = new Datasource(destIP, id, topic, sourceCount, grpcDest);
    myDatasource->readcsv(file);

    std::thread subThread(subscriber);
    std::thread pubThread(&Datasource::run, myDatasource);


    pubThread.join();
    subThread.join();
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

Datasource::Datasource(std::string destIPn, std::string idn, std::string topicn, std::string count,
                       std::string grpcDest) {
    csvArgs = new std::vector<Line *>;
    destIP = std::move(destIPn);
    id = std::move(idn);
    topic = std::move(topicn);
    //mqttName = "Datasource-" + id;
    mqttName = id;
    queue = new std::vector<queueLine *>;
    clock = 0;
    ackCounter = 0;
    sourceCount = std::stoi(count);
    grpcDest += ":8080";
    myClient = new GrpcClient(
            grpc::CreateChannel(
                    grpcDest,
                    grpc::InsecureChannelCredentials()
            )
    );

}

void Datasource::run() {
    try {
        std::cout << "publisher thread: " << mqttName << " with Topic: " << topic << std::endl;
        for (int i = 5; i > 0; i--) {
            std::cout << "starting in: " << i << std::endl;
            sleep(1);
        }
        int static doneWith = 0;
        for (int i = 0; i < csvArgs->size(); i++) {
            std::cout << "----------------new request------------------" << std::endl;

            Line *currentLine = csvArgs->at(i);

            sleep(currentLine->sleepTime);
            printVector();

            requestToEnter();

            bool isItDone = false;

            while (!isItDone) {


                if (allowedToEnter()) {
                    Reply myReply = myClient->storeData("1",
                                                        "Message of size: " + std::to_string(currentLine->fileSize),
                                                        std::to_string(currentLine->fileSize), "Message from: " + id);

                    std::cout << "Status: " << myReply.status() << "| Description: " << myReply.description()
                              << std::endl;

                    isItDone = true;
                    release();
                    printVector();
                }
                sleep(1);
            }

            std::cout << "finished request count: " << doneWith << "" << std::endl;
            ++doneWith;
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
    g_mutex.try_lock();
    std::cout << "Sending broadcast REQ" << std::endl;
    ++clock;

    std::string message = std::to_string(clock) + "|" + id + "|" + "REQ" + "|";
    publish(message);
    g_mutex.unlock();
    sleep(1);
}

void Datasource::allowToEnter(std::string requester) {

    g_mutex.try_lock();
    ++clock;
    std::string message =
            std::to_string(queue->front()->clock) + "|" + queue->front()->process + "|" + "ACK" + "|" + requester + "|";
    publish(message);
    g_mutex.unlock();
    sleep(1);
}

void Datasource::release() {
    g_mutex.try_lock();
    ++clock;
    std::string message =
            std::to_string(queue->front()->clock) + "|" + queue->front()->process + "|" + "REL" + "|" + id + "|";
    publish(message);
    g_mutex.unlock();
    sleep(1);
}

bool Datasource::allowedToEnter() {
    g_mutex.try_lock();
    auto t = queue[0].front();
    if (t->process == mqttName && ackCounter == sourceCount) {
        ackCounter = 0;
        std::cout << "i am allowed to enter" << std::endl;
        g_mutex.unlock();
        return true;
    }
    g_mutex.unlock();
    return false;


}


void Datasource::receive(std::string recMessage) {
    //PROCCLOCK|PROCID|EVENTTYP|REQUESTER

    std::string clockStr, procId, event, destOpt, rel;
    std::istringstream iss(recMessage);
    getline(iss, clockStr, '|');
    getline(iss, procId, '|');
    getline(iss, event, '|');
    if (event == "ACK") {
        getline(iss, destOpt, '|');
    }
    if (event == "REL") {
        getline(iss, rel, '|');
    }
    //g_mutex.try_lock();
    auto t = std::max(clock, std::stoi(clockStr));
    clock = t + 1;


    if (event == "REQ") {

        queueLine *ql = new queueLine();
        ql->clock = clock;
        ql->process = procId;
        queue->push_back(ql);
        allowToEnter(procId);
        ++messCounter;
        printVector();
    } else if (event == "ACK") {
        if (destOpt == id) {
            std::cout << "Got ACK for me! incrementing clock" << std::endl;
            ++ackCounter;
            ++messCounter;
            printVector();
        }
        std::cout << "Ignored this ACK. its not for me" << std::endl;


    } else if (event == "REL") {
        if (queue->at(0)->process == rel) {
            ++messCounter;
            queue->erase(queue->begin());
            printVector();

        }
    }


    auto sortRuleLambda = [](queueLine *s1, queueLine *s2) -> bool {
        if (s1->clock == s2->clock) {
            return s1;
        }
        return s1->clock < s2->clock;
    };
    if (queue->size() > 0) {
        std::sort(queue->begin(), queue->end(), sortRuleLambda);
    } else {
        std::cout << "nothing to sort.." << std::endl;
    }
}

void Datasource::printVector() {
    std::cout << "clock|Process" << std::endl;
    for (auto it : *queue) {
        std::cout << it->clock << "|" << it->process << std::endl;
    }
    std::cout << "Current clock: " << clock << std::endl;
    std::cout << "Current ACK count: " << ackCounter << std::endl;
    std::cout << "Current 3*(N-1): " << messCounter - (3 * 1) << std::endl;


}


void on_message(struct mosquitto *mosqSUB, void *obj, const struct mosquitto_message *msg) {
    printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);

    auto tmp = (char *) msg->payload;

    myDatasource->receive(tmp);

}

void on_connect(struct mosquitto *mosqtttest, void *obj, int rcttest) {
    std::cout << "on_connect called" << std::endl;
    if (rcttest != 0) {
        std::cout << "Conenction to broker failed" << std::endl;
        exit(-1);
    }
    mosquitto_subscribe(mosqtttest, NULL, myDatasource->getTopic().c_str(), 0);
}


void subscriber() {
    std::string brokerIP = myDatasource->getDestIp();
    int id = std::stoi(myDatasource->getId());
    std::cout << "starting subscriber thread: " << myDatasource->getMqttName() << " with Topic: "
              << myDatasource->getTopic() << std::endl;
    mosquitto_lib_init();
    std::string tmp = "sub-" + myDatasource->getId();
    mosqSUB = mosquitto_new(tmp.c_str(), true, &id);

    mosquitto_connect_callback_set(mosqSUB, on_connect);

    mosquitto_message_callback_set(mosqSUB, on_message);

    rcSUB = mosquitto_connect(mosqSUB, brokerIP.c_str(), 1883, 10);
    if (rcSUB) {
        throw "Could not connect to Broker";
    }

    mosquitto_loop_start(mosqSUB);
    printf("Thread is running...\n");
    mosquitto_loop_stop(mosqSUB, false);

    mosquitto_disconnect(mosqSUB);
    mosquitto_destroy(mosqSUB);
    mosquitto_lib_cleanup();

}

void publish(std::string mess) {
    int rc;
    struct mosquitto *mosq;
    mosquitto_lib_init();
    std::string tmp = "pub-" + myDatasource->getId();
    mosq = mosquitto_new(tmp.c_str(), true, NULL);

    rc = mosquitto_connect(mosq, myDatasource->getDestIp().c_str(), 1883, 10);
    if (rc != 0) {
        mosquitto_destroy(mosq);
        throw "Client could not connect to broker!";
    }

    mosquitto_publish(mosq, NULL, myDatasource->getTopic().c_str(), mess.length(), mess.c_str(), 1, false);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();
}