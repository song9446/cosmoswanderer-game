CPP_OPENSSL_OSX = -L/usr/local/opt/openssl/lib -L/usr/local/lib -I/usr/local/opt/openssl/include -I/usr/local/include

OUT = build/server
#SOURCES = server.cpp game_state.hpp
SOURCES = src/server.cpp
#all: build/server

$(OUT): $(SOURCES)
	$(CXX) $(CPP_OPENSSL_OSX) -std=c++11 -O3 $(SOURCES) -Isrc -o $(OUT) -L. -Lsrc -lpthread -luWS -lssl -lcrypto -lz -luv -lleveldb -llmdb

clean:
	rm -rf build/$(OUT)
