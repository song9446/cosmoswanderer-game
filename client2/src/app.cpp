#include <functional>
//#include "game_state.hpp"
#include "websocket_wrapper.cpp"
#include "sdlwrap.cpp"
#if defined(_EMSCRIPTEN__)
#include <emscripten.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <shlobj.h>
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
            //std::cout << e.x << "," << e.y << std::endl;
            //std::cout << e.x << "," << e.y << std::endl;
        });
        sdlwrap::MainLoop::run();
        /*
        start_loop([](void *arg){
            static LoadingPage loading_page;
            static LoginPage login_page;
            static ErrorPage fail_connect_server_page("Server died");
            App* app = reinterpret_cast<App*>(arg);
            sdlwrap::clearRenderer();
            switch(app->state){
            case State::NEED_LOGIN:
                login_page.render();
                app->state = State::FAIL_CONNECT_SERVER;
            break;
            case State::FAIL_CONNECT_SERVER:
                fail_connect_server_page.render();
                app->state = State::TRY_LOGIN;
            break;
            case State::TRY_LOGIN:
                loading_page.render(8);
            break;
            }
            sdlwrap::drawRenderer();
        }, 1, -1, this);
        */
    }
};

/*
            sdlwrap::drawRenderer();
            char hi[3] = "hi";
            app->socket.send(hi, 3);
            app->socket.recv([&](char* msg, int size){
                std::cout << msg << std::endl;
                stop_loop();
                //sdlwrap::close();
            });
*/
int main(){
    std::cout << "echo app start" << std::endl;
    App app = App::getInstance();
    app.run("13.124.198.237", 3000);
#if defined(_EMSCRIPTEN__)
    emscripten_exit_with_live_runtime();
#endif
    return 0;
}
