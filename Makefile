build:
	cmake . -B build
	cmake --build build

server:
	./build/server $(BUCKETS_CNT)

client:
	./build/client

work_load:
	./build/perf_test_client
