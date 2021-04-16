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
Exam was written within following time Constrains 08.03.2021 (8:00 Uhr) till 12.03.2021 (23:59), code passed the requierments.
More informations in 2020_ws_exam_met.pdf
