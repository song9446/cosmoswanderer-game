//#include <functional>
//#include "game_state.hpp"
//#define SDL_HINT_IME_INTERNAL_EDITING "1"
#include "websocket_wrapper.cpp"
#include "sdlwrap.cpp"
#if defined(_EMSCRIPTEN__)
#include <emscripten.h>
#else
#endif

void ugly_handle_socket_success(int fd, void* ud){
    auto state = reinterpret_cast<sdlwrap::Text*>(ud);
    state->set("server is alive", sdlwrap::Font::get<12>(), SDL_Color{0, 255, 0, 255});
}
void ugly_handle_socket_fail(int fd, void* ud){
    auto state = reinterpret_cast<sdlwrap::Text*>(ud);
    state->set("server may died", sdlwrap::Font::get<12>(), SDL_Color{255, 0, 0, 255});
}

class App{
private:
    enum class State: int {
        NEED_LOGIN, FAIL_CONNECT_SERVER, TRY_LOGIN, RECEIVING_LAST_STATE, RUNNING
    };
    static constexpr const char* APP_NAME = "CosmosWanderer";
    State state = State::NEED_LOGIN;
    //game_state::PartialMap* map = NULL;
    WebsocketWrapper socket;
    const char* title="CosmosWanderer";
    int width=800, height=600;
public:
    static App& getInstance() {
        static App instance;
        return instance;
    }
    void loadConfig(){
    }
    void setState(State s){
        state = s;
    }
    void run(const char* server_ip, int port){
        if(socket.connect(server_ip, port) != WebsocketWrapper::CODE::SUCCESS)
            state = State::FAIL_CONNECT_SERVER;

        sdlwrap::Window window(title, width, height);
        sdlwrap::Scene login_scene = loginScene(window);

        sdlwrap::MainLoop::run(60);
    }
    sdlwrap::Scene& loginScene(sdlwrap::Window& window){
        static sdlwrap::Scene scene(window);
        auto big_font = sdlwrap::Font::get<24>();
        auto normal_font = sdlwrap::Font::get<16>();
        auto color = SDL_Color{255, 255, 255, 255},
             yellow = SDL_Color{255, 255, 0, 255};
        auto state_label    = scene.add<sdlwrap::Text>(       0,          0,     "server connecting..",                normal_font,    yellow, sdlwrap::Align::TOP, sdlwrap::Align::LEFT);
        auto title    = scene.add<sdlwrap::Text>(       width/2,    height/2-90, "Cosmos Wanderer", big_font,       color, sdlwrap::Align::MIDDLE, sdlwrap::Align::CENTER);
        auto id_label = scene.add<sdlwrap::Text>(       width/2-80, height/2-30, "ID",              normal_font,    color, sdlwrap::Align::MIDDLE, sdlwrap::Align::LEFT);
        auto id_input = scene.add<sdlwrap::Input>(      width/2+80, height/2-30, 120,               normal_font,    color, sdlwrap::Align::MIDDLE, sdlwrap::Align::RIGHT);
        auto pw_label = scene.add<sdlwrap::Text>(       width/2-80, height/2+10, "PW",              normal_font,    color, sdlwrap::Align::MIDDLE, sdlwrap::Align::LEFT);
        auto pw_input = scene.add<sdlwrap::Input>(      width/2+80, height/2+10, 120,               normal_font,    color, sdlwrap::Align::MIDDLE, sdlwrap::Align::RIGHT, '*');
        auto login_button = scene.add<sdlwrap::Button>( width/2, height/2+50, 60,   20, "LOGIN", normal_font,    color, sdlwrap::Align::MIDDLE, sdlwrap::Align::CENTER);
        #if defined(_EMSCRIPTEN__)
        emscripten_set_socket_open_callback(state, ugly_handle_socket_state);
        emscripten_set_socket_error_callback(state, ugly_handle_socket_state);
        #else
        if(state == State::FAIL_CONNECT_SERVER) ugly_handle_socket_fail(-1, state_label);
        else ugly_handle_socket_success(-1, state_label);
        #endif
        login_button->listenKeyDown([](const SDL_KeyboardEvent& e){
            if(e.keysym.sym == SDLK_RETURN){
                printf("%s\n", "login..?");
            }
        });
        login_button->listenMouseDown([](const SDL_MouseButtonEvent& e){
            printf("%s\n", "login..?");
        });
        id_input->focusIn();
        return scene;
    }
};

// this is real main
void main_() {
    printf("%s\n", "game start");
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
