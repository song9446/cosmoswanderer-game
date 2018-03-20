CPP_OPENSSL_OSX = -L/usr/local/opt/openssl/lib -L/usr/local/lib -I/usr/local/opt/openssl/include -I/usr/local/include -I../server/src/game_state -Ithirparty/easywsclient -I/usr/include/SDL2
CPP_OPENSSL_WIN32 = -march=x86-64 -Wall -LC:/MinGW/lib -I/usr/local/opt/openssl/include -I/usr/local/include -I../server/src/game_state -Ithirparty/easywsclient -IC:/MinGW/x86_64-w64-mingw32/include/SDL2 -LC:/MinGW/x86_64-w64-mingw32/lib 
EMCC_ARG = -Wall -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_NET=2 -s USE_SDL_IMAGE=2 -s DISABLE_EXCEPTION_CATCHING=0 -s ASSERTIONS=1 -std=c++14 -I../server/src/game_state -O3  --preload-file asset

OUT_ROOT = build
WEB_OUT_ROOT = $(OUT_ROOT)/web
LINUX_OUT_ROOT = $(OUT_ROOT)/linux
WIN32_OUT_ROOT = $(OUT_ROOT)/win32/
#SOURCES = server.cpp game_state.hpp
SOURCES = src/app.cpp
#all: build/server

linux: $(SOURCES)
	mkdir -p $(LINUX_OUT_ROOT)
	cp -rf asset $(LINUX_OUT_ROOT)/
	g++ $(CPP_OPENSSL_OSX) -std=c++14 -O3 $(SOURCES) -Isrc -o $(LINUX_OUT_ROOT)/client -L. -Lsrc -lpthread -lGL -lSDL2 -lSDL2_net -lSDL2_image -lSDL2_ttf -lSDL2_mixer

web: $(SOURCES)
	mkdir -p $(WEB_OUT_ROOT)
	cp -rf src/shell/* $(WEB_OUT_ROOT)/
	emcc $(SOURCES) $(EMCC_ARG) -o $(WEB_OUT_ROOT)/client.js

win32: $(SOURCES)
	mkdir -p $(WIN32_OUT_ROOT)
	cp -rf src/win32_runtime_library/* $(WIN32_OUT_ROOT)/
	cp -rf asset $(WIN32_OUT_ROOT)/
	g++ $(CPP_OPENSSL_WIN32) -std=c++14 -O3 $(SOURCES) -Isrc -o $(WIN32_OUT_ROOT)/client -L. -Lsrc -lpthread -lSDL2 -lSDL2_net -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lwsock32 -lws2_32

clean:
	rm -rf $(OUT_ROOT)

