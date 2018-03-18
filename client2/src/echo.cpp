#include <iostream>
#include <cstdlib>
#include <emscripten.h>
#include <SDL2/SDL_net.h>
#include "websocket_wrapper.cpp"
// Since this is the client connection, we supply the ip of the server
// In this case, 127.0.0.1 which basically means "this computer"

void main_loop(void* arg){
    WebsocketWrapper* ww = reinterpret_cast<WebsocketWrapper*>(arg);
    char hi[3] = "hi";
    ww->send(hi, 3);
    ww->recv([&](char* msg, int size){
        std::cout << msg << std::endl;
        emscripten_cancel_main_loop();
    });
}
int main(){
    std::cout << "Client start" << std::endl;
    int port = 3000;
    WebsocketWrapper ww("13.124.198.237", port); 
    //emscripten_set_main_loop_arg(main_loop, &ww, -1, -1);
    emscripten_set_main_loop_arg([](void* arg){
        WebsocketWrapper* ww = reinterpret_cast<WebsocketWrapper*>(arg);
        char hi[3] = "hi";
        ww->send(hi, 3);
        ww->recv([&](char* msg, int size){
            std::cout << msg << std::endl;
            emscripten_cancel_main_loop();
        });
        emscripten_cancel_main_loop();
    }, &ww, -1, -1);
    std::cout << "Client end" << std::endl;
}
