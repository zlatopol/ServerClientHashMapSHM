# How to compile
```
cmake . -B build
cmake --build build
```
or
```
make build
```
<img width="643" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/ca7252e3-6fca-409b-9c59-bc80279f63c6">

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

<img width="735" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/c79cdb26-4cd3-4fb2-8a22-5f58b8005a89">

`make server BUCKETS_CNT=<bucket cnt>`

<img width="598" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/986dc458-8dc7-4452-98e1-d1534f14c53e">

`make client`

<img width="582" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/5168eaaf-fb0a-49b6-95c2-a47faa26f96a">

# Server
`./server <number of buckets in hash table>`

<img width="638" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/70487966-b9cd-4b19-800c-bb8cc0474def">


# Client
`./client`

<img width="680" alt="image" src="https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/f25d6a37-48f7-4a24-a632-57227f355036">


# Performance Test

`./perf_test_client (number_of_clients) (number_of_overall_requests)`

Output: `(id_of_client): (number_of_ops | time) (reads|inserts|deletes|ms)`
-1: overall time

## Results:
![telegram-cloud-photo-size-2-5440435529565984512-y](https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/0471a283-4213-4469-becb-b9fcd5657af7)
![telegram-cloud-photo-size-2-5440435529565984513-y](https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/ffe682b0-b71a-413e-9e44-3133c2788ca3)
![telegram-cloud-photo-size-2-5440435529565984514-y](https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/cd1d7a1e-6e85-4985-ae1b-0fda2c3deb71)
![telegram-cloud-photo-size-2-5440435529565984515-y](https://github.com/s3ponia/InMemoryAsyncHashMapSHM/assets/29208820/acdb9943-17a3-4c15-b010-eea06f4f3296)



