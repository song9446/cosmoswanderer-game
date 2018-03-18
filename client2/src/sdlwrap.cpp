#include "sdlwrap.hpp"
#include <stdio.h>

const char* UserDirectoryName = "CosmosWanderer";
#if defined(_EMSCRIPTEN__)
#include <emscripten.h>
#elif defined(_WIN32) || defined(_WIN64)
  EM_ASM(
    FS.mkdir('/ls');
    FS.mount(IDBFS, {}, '/ls');
    
    // sync from persisted state into memory and then
    // run the 'test' function
    FS.syncfs(true, function (err) {
      assert(!err);
      ccall('test', 'v');
    });
  );
/*
void func() {
  ..
  EM_ASM({
    FS.syncfs(.., function(err) {
      Module._continue();
    });
  });
  // might want to pause the main loop here, if one is running
}

extern "C" {
void EMSCRIPTEN_KEEPALIVE continue() {
  // data is now here, continue and use it
  // can resume main loop, if you are using one
}
*/
#include <shlobj.h>
TCHAR szFolderPath[MAX_PATH];
if (!SHGetSpecialFolderPath(NULL, szFolderPath, CSIDL_APPDATA, FALSE)) {
    // Uh-oh! An error occurred; handle it.
}
#else
"~/.config/"
#endif

namespace sdlwrap {
    template<typename ... Args>
    static void __error(const char* s, int l, const char* f, Args const & ... args){
        printf("%s:%d: error: ", s, l);
        printf(f, args...);
        printf("\n");
        fflush(stdout);
    }
#define error(...) __error(__FILE__, __LINE__, __VA_ARGS__)

    
    template<int size>
    TTF_Font*& Font::get(){
        static TTF_Font* font = TTF_OpenFont(DEFAULT_TTF_PATH, size);
        if(font == NULL) error("%s with %s", SDL_GetError(), DEFAULT_TTF_PATH);
        return font;
    }
    template<char... path, int size>
    TTF_Font*& Font::get(){
        static TTF_Font* font = TTF_OpenFont(path..., size);
        if(font == NULL) error("%s with %s", SDL_GetError(), path...);
        return font;
    }

    Texture::Texture(SDL_Renderer* renderer): renderer(renderer) { }
    Texture::Texture(SDL_Renderer* renderer, const char* path): renderer(renderer) {
        load(std::move(path));
    }
    Texture::Texture(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor): renderer(renderer) {
        load(std::move(text), std::move(font), std::move(textColor));
    }
    void Texture::load(const char* path){
        if(texture) free();
        SDL_Surface* loadedSurface = IMG_Load(path);
        if(loadedSurface == NULL) error("%s with %s", IMG_GetError(), path);
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if(texture == NULL) error("%s with %s", SDL_GetError(), path);
        w = loadedSurface->w;
        h = loadedSurface->h;
        SDL_FreeSurface( loadedSurface );
    }
    void Texture::load(const char* text, TTF_Font* font, SDL_Color textColor){
        if(texture) free();
        SDL_Surface* loadedSurface = TTF_RenderText_Solid(font, text, textColor);
        if(loadedSurface == NULL) error("%s with %s", TTF_GetError(), text);
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if(texture == NULL) error("%s", SDL_GetError());
        w = loadedSurface->w;
        h = loadedSurface->h;
        SDL_FreeSurface(loadedSurface);
    }
    void Texture::free(){ SDL_DestroyTexture(texture); }
    Texture::~Texture(){ if(texture) free(); }
    Texture& Texture::operator=(const Texture& fellow)
    {
        if(texture) SDL_DestroyTexture(texture);
        // avoid changing the name of the object
        return *this;
    }
    //void setColor( Uint8 red, Uint8 green, Uint8 blue ){}
    //void setBlendMode( SDL_BlendMode blending ){}
    //void setAlpha( Uint8 alpha ){}
    //void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE ){}

    void Window::initSDL() {
        // it will be initialized when first window created
        // * DO NOT MANUALLY CALL *
        static bool not_yet = true;
        if(not_yet){
            if(SDL_Init(SDL_INIT_VIDEO) < 0) error("%s", SDL_GetError());
            if(TTF_Init() < 0) error("%s", TTF_GetError());
            //if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) error("%s", IMG_GetError());
        }
    }
    void Window::render(){
        SDL_RenderClear(renderer);
        if(current_scene) current_scene->render();
        SDL_RenderPresent(renderer);
    }
    Window::Window(const char* title, int w, int h): Window(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h) {
    }
    Window::Window(const char* title, int x, int y, int w, int h, uint32_t flags): width(w), height(h){
        initSDL();
        if(!(window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags))) error("%s", SDL_GetError());
        if(!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))) error("%s", SDL_GetError());
        container.push_back(this);
    }
    Window::~Window(){
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        for(auto it = container.begin(); it != container.end(); ++it) if(*it == this){ container.erase(it); break; }
    }
    void Window::set(Scene* scene){ 
        current_scene = scene; 
    }
    void Window::set(Scene& scene){ 
        current_scene = &scene; 
    }
    void Window::handleMouseMotion(const SDL_MouseButtonEvent& e){
        if(mouseMotionHandler) mouseMotionHandler(e);
        if(current_scene) current_scene->handleMouseMotion(e);
    }
    void Window::handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(mouseButtonDownHandler) mouseButtonDownHandler(e);
        if(current_scene) current_scene->handleMouseButtonDown(e);
    }
    void Window::handleMouseButtonUp(const SDL_MouseButtonEvent& e){
        if(mouseButtonUpHandler) mouseButtonUpHandler(e);
        if(current_scene) current_scene->handleMouseButtonUp(e);
    }
    SDL_Renderer* Window::getRenderer(){ return renderer; }
    std::vector<Window*> Window::container;

    Component::Component(Scene& scene): renderer(scene.window->getRenderer()){
        scene.add(*this);
    }
    SDL_Renderer* Component::getRenderer(){ return renderer; }

    void Scene::render(){ 
        for(auto& c: components) c->render();
    }
    Scene::Scene(Window& w){ window = &w; }
    bool Scene::operator==(const Scene& rhs) const {
        return (this==&rhs);
    }
    void Scene::show(){ window->set(this); }
    void Scene::switchTo(Scene& other){ window->set(&other); }
    void Scene::add(Component& g){ components.push_back(&g); }
    //void Scene::remove(Component& g){ components.push_back(&g); }
    //Component& add(Component* component){ components.push_back(component); return *component;}
    //Component& add(Component& component){ components.push_back(&component); return component; }
    //friend void Window::render();

    Text::Text(Scene& scene, int x, int y, const char* str): Component(scene), texture(renderer) {
        set(str); 
        rect = {x, y, texture.getWidth(), texture.getHeight()};
    }
    Text::Text(Scene& scene, int x, int y, const char* str, TTF_Font* _font, SDL_Color _color): Component(scene), texture(renderer) {
        set(str, _font, _color); 
        rect = {x, y, texture.getWidth(), texture.getHeight()};
    }
    void Text::set(int _x, int _y){ rect.x = _x; rect.y = _y; }
    void Text::set(const char* str, TTF_Font* _font, SDL_Color _color){ 
        text = std::string(str); font = _font; color = _color;
        texture.load(text.c_str(), font, color);
    }
    void Text::set(const char* str){ 
        text = std::string(str); 
        texture.load(text.c_str(), font, color);
    }
    void Text::render(){
        texture.render(rect);
    }

    void MainLoop::run(int fps){
        auto loop_func = [](void* args){ 
            static SDL_Event e;
            for(Window*& w : Window::container) w->render(); 
            while(SDL_PollEvent(&e)) {
                switch(e.type){
                case SDL_MOUSEMOTION:
                    for(Window*& w : Window::container) w->handleMouseMotion(e.button); 
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    for(Window*& w : Window::container) w->handleMouseButtonDown(e.button); 
                    break;
                case SDL_MOUSEBUTTONUP:
                    for(Window*& w : Window::container) w->handleMouseButtonUp(e.button); 
                    break;
                }
            }
        };
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(loop_func, NULL, fps, -1);
#else
        auto now = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
        while(looping){
            t += std::chrono::milliseconds(1000/fps);
            loop_func(NULL);
            std::this_thread::sleep_until(t);
        }
#endif
    }
    void MainLoop::stop(){
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#else
        looping = false;
#endif
    }
    bool MainLoop::looping = false;

}
