# How to compile
```
cmake . -B build
cmake --build build
```
or
```
make build
```
![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/3131b583-756e-46b1-918d-41f36b6949ac)


# Task Description
* Server program:

     - Initialize a hash table of given size (command line)

     - Support insertion of items in the hash table

     - Hash table collisions are resolved by maintaining a linked list
for each bucket/entry in the hash table

     - Supports concurrent operations (multithreading) to perform
(insert, read, delete operations on the hash table)

     - Use readers-writer lock to ensure safety of concurrent
operations, try to optimize the granularity

     - Communicates with the client program using shared memory buffer
(POSIX shm)

* Client program:

     - Enqueue requests/operations (insert, read a bucket, delete) to
the server (that will operate on the the hash table) via shared memory
buffer (POSIX shm)
# Makefile
`make build`

![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/37abfedd-c2c7-4fa3-bb52-0bf8835bcf6b)


`make server BUCKETS_CNT=<bucket cnt>`

![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/1baa8f06-ee30-4e93-ba68-6e0a0878b534)


`make client`

![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/20502c21-b921-4bdf-a43a-332df088113c)


# Server
`./server <number of buckets in hash table>`

![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/1a12b611-38ba-4b80-921d-16e0f1d74725)


# Client
`./client`

![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/31c91be3-1d13-4b06-ae67-7afc7840f602)


# Performance Test

`./perf_test_client (number_of_clients) (number_of_overall_requests)`

Output: `(id_of_client): (number_of_ops | time) (reads|inserts|deletes|ms)`
-1: overall time

## Results:
![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/ff2c97c6-546e-450f-9c09-20ba5ba56a45)
![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/d56bcf7d-d4b1-455f-9c46-382893412495)
![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/37b3681c-b0eb-4859-95f5-b293a704438e)
![image](https://github.com/zlatopol/ServerClientHashMapSHM/assets/70473289/9a842a46-8d7d-4e2f-81d8-1f60429b6c13)



