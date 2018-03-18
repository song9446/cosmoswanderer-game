#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <vector>
#include <stdexcept>
#include <algorithm>
namespace sdl_wrapper{

//class MainLoop;

struct Font{
    static constexpr const char[] DEFAULT_TTF_PATH = "asset/font/ttf/NanumBarunGothic.ttf"
    template<int size>
    static TTF_Font& get(){
        static TTF_Font* font = TTF_OpenFont(DEFAULT_TTF_PATH, size);
        if(font == NULL) std::invalid_arugment(SDL_GetError());
        return font;
    }
    template<char... path, int size>
    static TTF_Font* get(){
        static TTF_Font* font = TTF_OpenFont(path, size);
        if(font == NULL) std::invalid_arugment(SDL_GetError());
        return font;
    }
};

class Window{
    SDL_Window* window;
    SDL_Renderer* renderer;
    Scene* current_scene;
    int width, height;
    static void initSDL() {
    // it will be initialized when first window created
    // * DO NOT MANUALLY CALL *
        static bool not_yet = true;
        if(not_yet){
            if(SDL_Init(SDL_INIT_VIDEO) < 0) throw std::invalid_argument(SDL_GetError());
            if(TTF_Init() < 0) throw std::invalid_argument(TTF_GetError());
            if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) throw std::invalid_argument(IMG_GetError());
        }
    }
    static std::vector<Window*> container;
    void render(){
        SDL_RenderClear(renderer);
        if(current_scene) current_scene->render(renderer);
        SDL_RenderPresent(renderer);
    }
public:
    Window(const char* title, int w, int h): width(w), height(h){
        Window(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h);
    }
    Window(const char* title, int x, int y, int w, int h, uint32_t flags=SDL_WINDOW_SHOWN): width(w), height(h){
        initSDL();
        if(!(window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags))) throw std::invalid_argument(SDL_GetError());
        if(!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))) throw std::invalid_argument(SDL_GetError());
        container.push_back(this);
    }
    ~Window(){
        SDL_DestoryWindow(window);
        SDL_DestroyRenderer(renderer);
        for(auto it = container.begin(); it != container.end(); ++it) if(*it == this){ container.erase(it); break; }
    }
    void set(Scene* scene){ 
        current_scene = scene; 
        current_scene->setWindow(this); 
    }
    friend class MainLoop;
};
std::vector<Window*> Window::container;

class Scene{
    Window* window;
    std::vector<GUIComponent*> gui_components
    void render(SDL_Renderer*& renderer){ 
        for(auto& c: gui_components) c.render(renderer);
    }
public:
    void setWindow(Window* w){ window=w; }
    void go(Scene* other){
        other->window = window;
        window.setCurrentScene(other);
    }
    void add(GUIComponent* component){ components.push_back(component); }
    friend void GUIComponent::render();
};

class GUIComponent{
public:
    virtual void render(SDL_Renderer*& ) = 0;
};

class Text: public GUIComponent {
    Texture texture;
    std::string text;
    TTF_Font* font = Font::get<16>();
    SDL_Color color = SDL_Color{0,0,0};
    bool need_texture_update=false;
    int x, y;
public:
    Text(int x, int y, const char* str): x(x), y(y) { set(str); }
    Text(int x, int y, const char* str, TTF_Font& _font, SDL_Color _color): x(x), y(y) { set(x, y, str, _font, _color); }
    void set(int _x, int _y){ x = _x; y = _y; }
    void set(const char* str, TTF_Font* _font, SDL_Color _color){ 
        text = std::string(str); font = _font; color = _color;
        need_texture_update = true; 
    }
    void set(const char* str){ 
        text = std::string(str); 
        need_texture_update = true; 
    }
    render(SDL_Renderer*& renderer){
        if(need_texture_update)
            texture.set(text.c_str(), font, color);
        texture.render(renderer, x, y);
    }
};

class Texture{
    SDL_Texture* texture = NULL;
    SDL_Rect rect;
    int w, h;
public:
    Texture(){
    }
    Texture(const char* path){
        load(std::move(path));
    }
    Texture(const char* text, TTF_Font* font=font16, SDL_Color textColor = SDL_Color{0,0,0}){
        load(std::move(text), std::move(font16), std::move(textColor));
    }
    void load(const char* path){
        if(texture) free();
        SDL_Surface* loadedSurface = IMG_Load(path);
        if( loadedSurface == NULL ) printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if( texture == NULL ) printf( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
        w = loadedSurface->w;
        h = loadedSurface->h;
        SDL_FreeSurface( loadedSurface );
    }
    void load(const char* text, TTF_Font* font=font16, SDL_Color textColor = SDL_Color{0,0,0}){
        if(texture) free();
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
        if(textSurface == NULL) printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
        texture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if(texture == NULL) printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        width = textSurface->w;
        height = textSurface->h;
        SDL_FreeSurface( textSurface );
    }
    void free(){ SDL_DestroyTexture(texture); }
    ~Texture(){ if(texture) free(); }
    Texture& operator=(const Texture& fellow)
    {
        if(texture) SDL_DestroyTexture(texture);
        // avoid changing the name of the object
        return *this;
    }
    //void setColor( Uint8 red, Uint8 green, Uint8 blue ){}
    //void setBlendMode( SDL_BlendMode blending ){}
    //void setAlpha( Uint8 alpha ){}
    //void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE ){}
    inline void render(SDL_Renderer*& renderer){ SDL_RenderCopy(renderer, texture, NULL, &rect); }
    inline void render(SDL_Renderer*& renderer, int x, int y){
        rect.x = x; rect.y = y;
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }
    inline void render(SDL_Renderer*& renderer, int x, int y, int w, int h){ 
        rect.x = x; rect.y = y; rect.w = w; rect.h = h;
        SDL_RenderCopy(renderer, texture, NULL, &rect); 
    }
    int getWidth(){ return w; }
    int getHeight(){ return h; }
};
/*
typedef const std::function<void (void*)> Collable;
bool looping = true;
void stop_loop(){
    #ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
    #else
    looping = false;
    #endif
}
template<typename T>
void start_loop(T func, int fps, int simulate_infinite_loop, void* arg){
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(func, arg, fps, simulate_infinite_loop);
    #else
    auto now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
    while(looping){
        t += std::chrono::milliseconds(1000/fps);
        func(arg);
        std::this_thread::sleep_until(t);
    }
    #endif
}
*/
class MainLoop {
    static bool looping;
    static void run(int fps=-1){
        auto loop_func = [](){ for(window const& w : Window::container) w.render(); };
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(loop_func, arg, fps, simulate_infinite_loop);
#else
        auto now = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
        while(looping){
            t += std::chrono::milliseconds(1000/fps);
            loop_func(arg);
            std::this_thread::sleep_until(t);
        }
#endif
    }
    static void stop(){
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#else
        looping = false;
#endif
    }
};
bool MainLoop::looping = false;

}
