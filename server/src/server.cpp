//#include <game_state/game_state.hpp>
#include <iostream>
#include <uWS/uWS.h>
#include <thread>
#include <server_config.hpp>
//#include "leveldb/db.h"
#include "db_wrapper.hpp"


class UserManager {
public:
    enum class JOIN_RESULT: int {
        SUCCESS=0, ID_ALREADY_EXIST=-1, WRONG_SECRET=-2
    };
    enum class LOGIN_RESULT: int {
        SUCCESS=0, NONEXIST_ID=-1, WRONG_SECRET=-2
    };
    struct Userdata {
        char mail[128];
        char secret[128];
    };
private:
    LMDBWrapper db;
    const static int chunk_size = sizeof(struct Userdata);
public:
    UserManager(const char* path): db(path){ 

    }
    ~UserManager(){
    }
    JOIN_RESULT join(MDB_val id, MDB_val secret){
        if(!db.exist(id))
            db.put(id, secret);
        else return JOIN_RESULT::ID_ALREADY_EXIST;
    }
    LOGIN_RESULT login(MDB_val id, MDB_val secret){
        struct Userdata ud;
        if(!db.get(id, &ud))
            return LOGIN_RESULT::NONEXIST_ID;
        if(!memcmp(ud.secret, secret.mv_data, secret.mv_size))
            return LOGIN_RESULT::SUCCESS;
        else
            return LOGIN_RESULT::WRONG_SECRET;
    }
};


class Packet {
public:
    enum class OP: unsigned int{
        ECHO=0, JOIN=1, LOGIN=2  
    };
    OP op;
    char* data;
    size_t length;
    bool should_free_memory = false;
    Packet(char *message, size_t length){
        op = *reinterpret_cast<OP*>(message);
        data = message+sizeof(OP);
        length = length - sizeof(OP);
    }
private:
    template <class T, class ...Rest>
    int sumUpLength(T& t, Rest... rest){
        return sizeof(t) + sumUpLength(rest...);
    }
    template <class ...Rest>
    int sumUpLength(char* t, int l, Rest... rest){
        return l + sumUpLength(rest...);
    }
public:
    template <class ...Rest>
    Packet(OP _op, Rest... rest){
        op = _op;
        length = sumUpLength(rest...);
        data = new char[length];
        should_free_memory = true;
        Packet(rest...);
    }
    template <class ...Rest>
    Packet(char *message, size_t length, Rest... rest){
        memcpy(data, message, length);
        data += length;
        Packet(rest...);
    }
    template <class T, class ...Rest>
    Packet(OP _op, T& t, Rest... rest){
        memcpy(data, &t, sizeof(t));
        data += sizeof(t);
        Packet(rest...);
    }
    Packet(){
        data -= length;
    }
    ~Packet(){
        if(should_free_memory) delete[] data;
    }
    uint8_t nextUInt8(){
        if(!length) return 0;
        data += sizeof(uint8_t);
        length -= sizeof(uint8_t);
        return *(reinterpret_cast<uint8_t*>(data)-1);
    }
    int nextInt(){
        if(!length) return 0;
        data += sizeof(int);
        length -= sizeof(int);
        return *(reinterpret_cast<int*>(data)-1);
    }
    MDB_val nextMDBVal(){
        if(!length) return MDB_val();
        MDB_val b{*reinterpret_cast<size_t*>(data), data + sizeof(size_t)};
        data += sizeof(size_t) + b.mv_size;
        return b;
    } 
};

int main()
{
    uWS::Group<uWS::SERVER> *login_group = NULL;
    //logout_group->addAsync();
    //login_group->addAsync();
    std::thread login_thread([&login_group]{
        uWS::Hub h;
        login_group = &h.getDefaultGroup<uWS::SERVER>();
        login_group->onMessage([&](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
            Packet packet(message, length);
            switch(packet.op){
            case Packet::OP::ECHO:
                ws->send(packet.data, packet.length, opCode);
                break;
            }
        });
        h.getDefaultGroup<uWS::SERVER>().listen(uWS::TRANSFERS);
        h.run();
    });

    UserManager user_manager("test");
    uWS::Hub h;
    uWS::Group<uWS::SERVER> *logout_group = h.createGroup<uWS::SERVER>();
    login_group->onMessage([&login_group, &user_manager](uWS::WebSocket<uWS::SERVER> *ws, char *message, size_t length, uWS::OpCode opCode) {
        Packet packet(message, length);
        switch(packet.op){
        case Packet::OP::ECHO:
            ws->send(packet.data, packet.length, opCode);
            break;
        case Packet::OP::LOGIN:
            if(user_manager.login(packet.nextMDBVal(), packet.nextMDBVal()) == UserManager::LOGIN_RESULT::SUCCESS){
                uint8_t one = 1;
                ws->transfer(login_group);
                ws->send(reinterpret_cast<const char*>(&one), 1, opCode);
            }
            else{
                uint8_t zero = 1;
                ws->send(reinterpret_cast<const char*>(&zero), 1, opCode);
            }
            break;
        }
    });
    if (h.listen(server_config::SERVER_PORT, nullptr, uS::REUSE_PORT, logout_group)) {
        std::cout << "server is running on " << server_config::SERVER_PORT << std::endl;

    } else {
        std::cout << "FAILURE: Cannot listen to same port twice!" << std::endl;
        exit(-1);
    }
    h.run();
    login_thread.join();
    std::cout << "server end!" << std::endl; 
}
