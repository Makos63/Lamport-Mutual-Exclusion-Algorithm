#### **Exam exercise for WS20-21 in Distributed Systems**

**To start execute**
```
docker-compose build ; docker-compose up 
```
in the main directory of the project

**To stop**
```
ctrl+c
```

**To adjust number of Datasources:**
1. Add or delete in docker-compose file:
``` 
 dsX:
     build:
       context: Datasource
       dockerfile: dockerDatasource
     environment:
       DesIP: IPMosquitto
       GrpcDest: IPDatastore
       Csv: config.csv
       id: X
       sourceCount: Y
     networks:
       myNetwork:
         ipv4_address: Z
     depends_on:
       - mosquitto
```
2. replace X new id is; 
3. replace Y (also others by Datasources) the sourceCount to count of all Datasources
4. replace Z with free ip address from network 172.20.0.0/22


**To adjust number of Datasources(fast):**
You may also second compose file with 5 Datasources















**Informations about project constrains**

Exam was written within following time Constrains 08.03.2021 (8:00) till 12.03.2021 (23:59), code passed the requirements.



The main task was to implement Lamport's mutual exclusion algorithm for database access. The database access was possible only with Thrift, gRPC or TCP with Protobuf. Other requirements were: horizontal scalability for more than three datasources which produce data and try to save it and automated docker deployment. 

![alt text](https://github.com/Makos63/Lamport-Mutual-Exclusion-Algorithm/blob/master/starting_arch.png?raw=true)
Datenquelle(datasource), Gemeinsamer Datenspeicher(datastore)


Used communication technologies:
- gRPC (C++) for contact with datastore
- Eclipse Mosquitto (C++) for Lamports alg. implementation


This solution's problems are: (1) race condition which was not solved in given time constrains, however it fulfills acceptance exam criteria, (2) lack of refactoring which was not quite possible with old Eclipse Mosquitto which is not further developed.



Detailed informations in 2020_ws_exam_met.pdf

Solution is inspired by: M. van Steen and A.S. Tanenbaum, Distributed Systems, 3rd ed.
