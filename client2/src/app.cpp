//#include <functional>
//#include "game_state.hpp"
//#define SDL_HINT_IME_INTERNAL_EDITING "1"
#include "websocket_wrapper.cpp"
#include "sdlwrap.cpp"
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
#include <unistd.h>
#endif

void ugly_handle_socket_success(int fd, void* ud){
    sdlw::Text* state = reinterpret_cast<sdlw::Text*>(ud);
    state->set("server is alive", sdlw::Font::get<12>(), SDL_Color{0, 255, 0, 255});
}
void ugly_handle_socket_fail(int fd, int t, const char* msg, void* ud){
    sdlw::Text* state = reinterpret_cast<sdlw::Text*>(ud);
    state->set((std::string("server may died: ")+ msg).c_str(), sdlw::Font::get<12>(), SDL_Color{255, 0, 0, 255});
}

/*
struct GameScene {
    sdlw::Scene scene;
    sdlw::Worm
    SDL_Font* small_font = sdlw::Font::get<12>();
    SDL_Font* normal_font = sdlw::Font::get<16>();
    SDL_Font* big_font = sdlw::Font::get<32>();
    
    GameScene(){
    }
};
*/

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

        sdlw::MainLoop mainloop;
        sdlw::Window window(&mainloop, title, width, height);
        sdlw::Scene* test_scene = testScene(&window);
        sdlw::Scene* login_scene = loginScene(&window, test_scene);
        //login_scene->show();

        mainloop.run(60);
    }
    sdlw::Scene* testScene(sdlw::Window* window){
        auto spritesheet = new sdlw::SpriteSheet(window->getRenderer(), "asset/spritesheet/test.csv");
        sdlw::Scene* scene = new sdlw::Scene(window);
        auto big_font = sdlw::Font::get<36>();
        auto color = SDL_Color{255, 255, 255, 255};
        auto title    = new sdlw::Text(scene,       width/2,    height/2, "Test!!!!!!가나다라마바사!!", big_font,       color, sdlw::Align::MIDDLE, sdlw::Align::CENTER);
        auto imgspt = new sdlw::SpriteImage(scene, spritesheet->get("asset/sprite/test2.png"), 0, 0, 90, NULL);//, sdlw::Align::MIDDLE, sdlw::Align::CENTER);
        auto img = new sdlw::Image(scene, "asset/sprite/test2.png", 200, 200);
        return scene;
    }
    sdlw::Scene* loginScene(sdlw::Window* window, sdlw::Scene* nextscene){
        sdlw::Scene* scene = new sdlw::Scene(window);
        auto big_font = sdlw::Font::get<24>();
        auto normal_font = sdlw::Font::get<16>();
        auto color = SDL_Color{255, 255, 255, 255},
             yellow = SDL_Color{255, 255, 0, 255};
        auto state_label    = new sdlw::Text(scene, 0,          0,     "server connecting..",                normal_font,    yellow, sdlw::Align::TOP, sdlw::Align::LEFT);
        auto title    = new sdlw::Text(scene,       width/2,    height/2-90, "Cosmos Wanderer", big_font,       color, sdlw::Align::MIDDLE, sdlw::Align::CENTER);
        auto id_label = new sdlw::Text(scene,       width/2-80, height/2-30, "ID",              normal_font,    color, sdlw::Align::MIDDLE, sdlw::Align::LEFT);
        auto id_input = new sdlw::Input(scene,      width/2+80, height/2-30, 120,               normal_font,    color, sdlw::Align::MIDDLE, sdlw::Align::RIGHT);
        auto pw_label = new sdlw::Text(scene,       width/2-80, height/2+10, "PW",              normal_font,    color, sdlw::Align::MIDDLE, sdlw::Align::LEFT);
        auto pw_input = new sdlw::Input(scene,      width/2+80, height/2+10, 120,               normal_font,    color, sdlw::Align::MIDDLE, sdlw::Align::RIGHT, '*');
        auto login_button = new sdlw::Button(scene, width/2, height/2+50, 60,   20, "LOGIN", normal_font,    color, sdlw::Align::MIDDLE, sdlw::Align::CENTER);
        #if defined(__EMSCRIPTEN__)
        emscripten_set_socket_open_callback((void*)state_label, ugly_handle_socket_success);
        emscripten_set_socket_error_callback((void*)state_label, ugly_handle_socket_fail);
        #else
        if(state == State::FAIL_CONNECT_SERVER) ugly_handle_socket_fail(-1, -1, "server may died", state_label);
        else ugly_handle_socket_success(-1, state_label);
        #endif
        login_button->listenKeyDown([&nextscene](const SDL_KeyboardEvent& e){
            if(e.keysym.sym == SDLK_RETURN){
                printf("%s\n", "login..?");
                nextscene->show();
            }
        });
        login_button->listenMouseDown([&nextscene](const SDL_MouseButtonEvent& e){
            printf("%s\n", "login..?");
            nextscene->show();
        });
        id_input->focusIn();
        return scene;
    }
};

// this is real main
extern "C" {void main_() {
    printf("%s\n", "game start");
    App app = App::getInstance();
    app.run("13.124.198.237", 3000);
#if defined(__EMSCRIPTEN__)
    EM_ASM(
        FS.syncfs(function (err) {
            assert(!err);
            //ccall('success', 'v');
        });
    );
#endif
}}

// this is shadow main
// * DO NOT TOUCH *
int main(int argc, char **argv){
    printf("P: %s\n", SDL_GetPrefPath("My Company", "My Awesome SDL 2 Game"));
    printf("P: %s\n", SDL_GetBasePath());
#if defined(__EMSCRIPTEN__)
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
