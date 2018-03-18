//#include <functional>
//#include "game_state.hpp"
#include "websocket_wrapper.cpp"
#include "sdlwrap.cpp"
#if defined(_EMSCRIPTEN__)
#include <emscripten.h>
#else
#endif


class App{
private:
    enum class State {
        NEED_LOGIN, FAIL_CONNECT_SERVER, TRY_LOGIN, RECEIVING_LAST_STATE, RUNNING
    };
    static constexpr const char* APP_NAME = "CosmosWanderer";
    State state = State::NEED_LOGIN;
    //game_state::PartialMap* map = NULL;
    WebsocketWrapper socket;
    const char* title="CosmosWanderer";
    int width=640, height=480;
public:
    static App& getInstance() {
        static App instance;
        return instance;
    }
    void loadConfig(){
    }
    void run(const char* server_ip, int port){
        if(socket.connect(server_ip, port) != WebsocketWrapper::CODE::SUCCESS)
            state = State::FAIL_CONNECT_SERVER;
        sdlwrap::Window window(title, width, height);
        sdlwrap::Scene scene1(window);
        sdlwrap::Text text(scene1, 10, 10, "hi");
        sdlwrap::Text text2(scene1, 22, 20, "oh oh..");
        sdlwrap::Scene scene2(window);
        sdlwrap::Text text3(scene1, 10, 10, "hiyo");
        sdlwrap::Text text4(scene1, 22, 20, "no no..");
        scene1.show();
        text.listenMouseMotion([](const SDL_MouseButtonEvent& e){
            std::cout << "hi" << std::endl;
        });
        sdlwrap::MainLoop::run();
    }
};

// this is real main
void main_() {
    std::cout << "echo app start" << std::endl;
    App app = App::getInstance();
    app.run("13.124.198.237", 3000);
#if defined(_EMSCRIPTEN__)
    EM_ASM(
        FS.syncfs(function (err) {
            assert(!err);
            ccall('success', 'v');
        });
    );
#endif
}

// this is shadow main
// * DO NOT TOUCH *
int main(){
#if defined(_EMSCRIPTEN__)
    EM_ASM(
        FS.mkdir('/IDBFS');
        FS.mount(IDBFS, {}, '/IDBFS');
        FS.syncfs(true, function (err) {
            assert(!err);
            ccall('main_', 'v');
        });
    );
    emscripten_exit_with_live_runtime();
#else
    main_();
#endif
    return 0;
}
