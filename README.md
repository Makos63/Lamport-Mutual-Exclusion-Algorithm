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
       DesIP: 172.20.0.110
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
