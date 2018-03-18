#include <iostream>
#include <functional>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL_net.h>
#include <vector>

const int INITIAL_BUF_SIZE = 1024;
class WebsocketWrapper {
private:
    typedef const std::function<void (char*, int)> Collable;
    TCPsocket socket;
    std::vector<char> buf;
public:
    enum class CODE: int {
        SUCCESS=0, FAIL_OPEN_PORT=1, FAIL_CONNECT=2, 
    };
    WebsocketWrapper(){ }
    CODE connect(const char* addr, int port){
        IPaddress ipAddress;
        if(-1 == SDLNet_ResolveHost(&ipAddress, addr, port)){
            std::cout << "Failed to open port : " << port << std::endl;
            return CODE::FAIL_OPEN_PORT;
        }
        socket = SDLNet_TCP_Open(&ipAddress);
        if(socket == nullptr){
            std::cout << "Failed to connect to port : " << port
                <<" \n\tError : " << SDLNet_GetError()
                << std::endl;
            return CODE::FAIL_CONNECT;
        }
        buf.resize(INITIAL_BUF_SIZE);
        return CODE::SUCCESS;
    }
    int send(void* msg, int size){
        return SDLNet_TCP_Send(socket, msg, size);
    }
    int recv(Collable& collable){
        int len = 0, t=0;
        while((t = SDLNet_TCP_Recv(socket, &buf[len], buf.size()-len))>0){
            if(t == buf.size()-len)
                buf.resize(2*buf.size());
            len += t;
        }
        if(len) collable(&buf[0], len);
        return len;
    }
};

#else
#include "../thirdparty/easywsclient/easywsclient.hpp"
#include "../thirdparty/easywsclient/easywsclient.cpp"
class WebsocketWrapper {
private:
    typedef const std::function<void (char*, int)> Collable;
    easywsclient::WebSocket::pointer ws;
public:
    enum class CODE: int{
        SUCCESS=0, FAIL_OPEN_PORT=1, FAIL_CONNECT=2, 
    };
    WebsocketWrapper(){ }
    CODE connect(const char* addr, int port){
        std::string addr_str(addr);
        addr_str += ":" + std::to_string(port);
        addr_str = "ws://" + addr_str;
        ws = easywsclient::WebSocket::from_url(addr_str);
        if(ws != NULL)
            return CODE::SUCCESS;
        else return CODE::FAIL_CONNECT;
    }
    int send(void* msg, int size){
        ws->sendBinary(std::string(reinterpret_cast<char*>(msg), size));
        ws->poll();
        return size;
    }
    int recv(Collable& collable){
        ws->poll();
        int len = 0;
        ws->dispatchBinary([&collable, &len](std::vector<uint8_t> message){
            collable(reinterpret_cast<char*>(&message[0]), message.size());
            len = message.size();
        });
        return len;
    }
};
#endif
