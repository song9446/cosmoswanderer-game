#include "sdlwrap.hpp"
#include <stdio.h>
#include <fstream>
#if defined(_EMSCRIPTEN__)
#include <emscripten.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <shlobj.h>
#else
#include <chrono>
#include <thread>
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

#if defined(_EMSCRIPTEN__)
    // idb must be mounted on /IDBFS 
    // It is happened on shadow main(see the main function)
    const char* UserDataSaver::user_directory_path = "/IDBFS/";
#elif defined(_WIN32) || defined(_WIN64)
    TCHAR szFolderPath[256];
    if (!SHGetSpecialFolderPath(NULL, szFolderPath, CSIDL_APPDATA, FALSE)) 
        error("%s", "cannot get Windows User data directory");
    const char* UserDataSaver::user_directory_path = szFolderPath;
#else
    const char* UserDataSaver::user_directory_path = "~/.config/";
#endif
    int UserDataSaver::save(const char* relative_path, void* data, size_t size){ 
        std::string path = user_directory_path;
        path += relative_path;
        FILE* f = fopen(path.c_str(), "w");
        if(f == NULL){
            error("file open at %s failed", path.c_str()); 
            return -1;
        }
        if(size != fwrite(data, 1, size, f)){
            error("file write at %s failed", path.c_str()); 
            fclose(f);
            return -2;
        }
        fclose(f);
        return 0;
    }
    int UserDataSaver::load(const char* relative_path, void* data, size_t size){ 
        std::string path = user_directory_path;
        path += relative_path;
        FILE* f = fopen(path.c_str(), "w");
        if(f == NULL){
            error("file open at %s failed", path.c_str()); 
            return -1;
        }
        if(size != fread(data, 1, size, f)){
            error("file read at %s failed", path.c_str()); 
            return -2;
        }
        fclose(f);
        return 0;
    }

    
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

    Texture::Texture(SDL_Renderer* renderer, Align align_v, Align align_h): renderer(renderer), align_v(align_v), align_h(align_h) { }
    Texture::Texture(SDL_Renderer* renderer, const char* path, Align align_v, Align align_h): renderer(renderer), align_v(align_v), align_h(align_h) {
        load(std::move(path));
    }
    Texture::Texture(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor, Align align_v, Align align_h): renderer(renderer), align_v(align_v), align_h(align_h) {
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
        SDL_Surface* loadedSurface = TTF_RenderUTF8_Blended(font, text, textColor);
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
            not_yet = false;
            if(SDL_Init(SDL_INIT_VIDEO) < 0) error("%s", SDL_GetError());
            if(TTF_Init() < 0) error("%s", TTF_GetError());
            //if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) error("%s", IMG_GetError());
        }
    }
    void Window::render(){
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
        if(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) error("%s", SDL_GetError());
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
    void Window::handleTextInput(const SDL_TextInputEvent& e){
        if(textInputHandler) textInputHandler(e);
        if(current_scene) current_scene->handleTextInput(e);
    }
    void Window::handleKeyDown(const SDL_KeyboardEvent& e){
        if(keyDownHandler) keyDownHandler(e);
        if(current_scene) current_scene->handleKeyDown(e);
    }
    void Window::handleKeyUp(const SDL_KeyboardEvent& e){
        if(keyUpHandler) keyUpHandler(e);
        if(current_scene) current_scene->handleKeyUp(e);
    }
    SDL_Renderer* Window::getRenderer(){ return renderer; }
    std::vector<Window*> Window::container;

    Component::Component(Scene* scene): renderer(scene->window->getRenderer()){
       //scene.add(this);
    }
    SDL_Renderer* Component::getRenderer(){ return renderer; }

    void Scene::render(){ 
        for(auto& c: components) c->render();
    }
    Scene::Scene(Window& w){
        window = &w; 
        if(!w.current_scene) window->set(this);
    }
    bool Scene::operator==(const Scene& rhs) const {
        return (this==&rhs);
    }
    void Scene::show(){ 
        window->set(this); 
    }
    void Scene::switchTo(Scene* other){ other->show(); }
    void Scene::add(Component* g){ components.push_back(g); }

    template<class ComponentType, typename... Args>
    ComponentType* Scene::add(Args... args){
        ComponentType* c = new ComponentType(this, args...);
        add(c);
        return c;
    }
    Scene::~Scene(){
        /*for(auto& c : components){
            delete c;
        }*/
    }

    //void Scene::remove(Component& g){ components.push_back(&g); }
    //Component& add(Component* component){ components.push_back(component); return *component;}
    //Component& add(Component& component){ components.push_back(&component); return component; }
    //friend void Window::render();

    Text::Text(Scene* scene, int x, int y, const char* str, TTF_Font* _font, SDL_Color _color, Align align_v, Align align_h): Component(scene), texture(renderer, align_v, align_h), align_v(align_v), align_h(align_h) {
        set(str, _font, _color); 
        set(x, y);
    }
    void Text::set(int _x, int _y){ 
        rect.x = _x - (align_h==Align::CENTER)*rect.w/2 - (align_h==Align::RIGHT)*(rect.w); 
        rect.y = _y - (align_v==Align::MIDDLE)*rect.h/2 - (align_v==Align::BOTTOM)*(rect.h); 
    }
    void Text::set(const char* str, TTF_Font* _font, SDL_Color _color){ 
        font = _font; color = _color;
        set(str);
    }
    void Text::set(const char* str){ 
        if(str && str[0] != '\0'){
            text = std::string(str); 
            texture.load(text.c_str(), font, color);
            rect.w = texture.getWidth(); 
            rect.h = texture.getHeight();
        }
        else{
            //texture.free();
            rect.w = 0;
        }
    }
    void Text::render(){
        texture.render(rect);
    }

    Input::Input(Scene* scene, int x, int y, int w, TTF_Font* _font, SDL_Color _color, Align align_v, Align align_h, char overlap): Text::Text(scene, x, y, NULL, _font, _color, align_v, Align::LEFT), bg_align_h(align_h), overlap(overlap) {
        cursor_poses.push_back(0);
        bg_rect = {x, y, w, TTF_FontHeight(_font)+3};
        cursor_rect = {x+rect.w, y, 1, TTF_FontHeight(_font)};
        underbar_rect = {x, y+bg_rect.h-1, w, 1};
        set(x, y);
    }
    void Input::focusIn(){ 
        focused = this; 
    };
    void Input::focusOut(){ 
        focused = NULL; 
    };
    void Input::handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(isCollision(bg_rect, e.x, e.y)){
            focusIn();
            int x = e.x - rect.x;
            int l = cursor_poses.size(), 
                p=2,
                i = l/p;
            while(p<l){
                p *= 2;
                if(x > cursor_poses[i])
                    i += l/p;
                else if(x < cursor_poses[i])
                    i -= l/p;
            }
            if(i < l-1 && std::abs(cursor_poses[i] - x) > std::abs(cursor_poses[i+1] - x))
                i+=1;
            if(i > 0 && std::abs(cursor_poses[i] - x) > std::abs(cursor_poses[i-1] - x))
                i-=1;
            setCursor(i);
            if(mouseButtonDownHandler) mouseButtonDownHandler(e); 
        }
    }
    void Input::setCursor(int index){
        counter = 0;
        cursor = index;
        cursor_rect.x = rect.x + cursor_poses[index];
    }
    void Input::handleTextInput(const SDL_TextInputEvent& e){
        if(focused == this){ 
            for(int i=0, l=strlen(e.text), t; i<l; ++i){
                TTF_GlyphMetrics(font, overlap=='\0'? e.text[i] : overlap, NULL, NULL, NULL, NULL, &t);
                cursor_poses.push_back(cursor_poses.back() + t);
                text.insert(cursor+i, 1, overlap=='\0'? e.text[i] : overlap);
            }
            real_text.insert(cursor, e.text);
            set(text.c_str());
            setCursor(cursor+1);
        }
        if(textInputHandler) textInputHandler(e); 
    }
    void Input::handleKeyDown(const SDL_KeyboardEvent& e){
        if(focused == this){
            if(e.keysym.sym == SDLK_BACKSPACE && cursor > 0){
                cursor_poses.erase(cursor_poses.begin() + cursor);
                text.erase(cursor-1, 1);
                real_text.erase(cursor-1, 1);
                setCursor(cursor-1);
                set(text.c_str());
            }
            if(e.keysym.sym == SDLK_DELETE && cursor < cursor_poses.size()-1){
                cursor_poses.erase(cursor_poses.begin() + cursor + 1);
                text.erase(cursor, 1);
                real_text.erase(cursor, 1);
                //setCursor(cursor-1);
                set(text.c_str());
            }
            if(e.keysym.sym == SDLK_RIGHT && cursor < cursor_poses.size()-1)
                setCursor(cursor+1);
            if(e.keysym.sym == SDLK_LEFT && cursor > 0)
                setCursor(cursor-1);
        }
    }
    void Input::set(int _x, int _y){
        _x -= (bg_align_h==Align::CENTER)*bg_rect.w/2 + (bg_align_h==Align::RIGHT)*(bg_rect.w); 
        _y -= (align_v==Align::MIDDLE)*bg_rect.h/2 + (align_v==Align::BOTTOM)*(bg_rect.h); 
        rect.x = _x;
        rect.y = _y;
        bg_rect.x = rect.x;
        bg_rect.y = rect.y;
        underbar_rect.x = rect.x;
        underbar_rect.y = rect.y + bg_rect.h;
        cursor_rect.y = rect.y;
        setCursor(cursor);
    }
    void Input::set(const char* str){ 
        Text::set(str);
    }
    void Input::set(const char* str, TTF_Font* _font, SDL_Color _color){ 
        Text::set(str, _font, _color);
    }
    void Input::render(){
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        if(focused == this && ++counter % 60 < 30)
            SDL_RenderFillRect(renderer, &cursor_rect);
        //SDL_RenderFillRect(renderer, &bg_rect);
        SDL_RenderFillRect(renderer, &underbar_rect);
        texture.render(rect);
    }
    Input* Input::focused = NULL;

    Button::Button(Scene* scene, int x, int y, int w, int h, const char* str, TTF_Font* _font, SDL_Color _color, Align align_v, Align align_h): Text::Text(scene, x, y, str, _font, _color, Align::MIDDLE, Align::CENTER), bg_align_h(align_h), bg_align_v(align_v){
        bg_rect = {x, y, w, h};
        set(x, y);
    }
    void Button::set(int x, int y){
        x -= (bg_align_h==Align::CENTER)*bg_rect.w/2 + (bg_align_h==Align::RIGHT)*(bg_rect.w); 
        y -= (bg_align_v==Align::MIDDLE)*bg_rect.h/2 + (bg_align_v==Align::BOTTOM)*(bg_rect.h); 
        bg_rect.x = x;
        bg_rect.y = y;
        rect.x = x + (bg_rect.w - rect.w)/2; 
        rect.y = y + (bg_rect.h - rect.h)/2; 
    }
    void Button::handleMouseMotion(const SDL_MouseButtonEvent& e){
        if(isCollision(bg_rect, e.x, e.y)){
            bg_color.a = down? 0 : 64;
            if(mouseMotionHandler) mouseMotionHandler(e);
        } else{
            bg_color.a = 0;
            down = false;
        }
    }
    void Button::handleMouseButtonUp(const SDL_MouseButtonEvent& e){
        down = false;
        if(isCollision(bg_rect, e.x, e.y)){
            bg_color.a = 64;
            if(mouseButtonUpHandler) mouseButtonUpHandler(e);
        }
    }
    void Button::handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(isCollision(bg_rect, e.x, e.y)){
            down = true;
            bg_color.a = 0;
            if(mouseButtonDownHandler) mouseButtonDownHandler(e);
        }
    }
    void Button::render(){
        SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, (Uint8)bg_color.a);
        SDL_RenderFillRect(renderer, &bg_rect);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRect(renderer, &bg_rect);
        texture.render(rect);
    }

    void MainLoop::run(int fps){
        auto loop_func = [](void* args){ 
            static SDL_Event e;
            static std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
            if(std::chrono::system_clock::now() >= t){
                for(Window*& w : Window::container) w->render(); 
                t += std::chrono::milliseconds(1000/60);
            }
            SDL_StartTextInput();
            while(SDL_PollEvent(&e)) {
                switch(e.type){
                case SDL_QUIT: 
                    MainLoop::stop();
                    break;
                case SDL_MOUSEMOTION:
                    for(Window*& w : Window::container) w->handleMouseMotion(e.button); 
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    for(Window*& w : Window::container) w->handleMouseButtonDown(e.button); 
                    break;
                case SDL_MOUSEBUTTONUP:
                    for(Window*& w : Window::container) w->handleMouseButtonUp(e.button); 
                    break;
                case SDL_KEYDOWN:
                    for(Window*& w : Window::container) w->handleKeyDown(e.key); 
                    break;
                case SDL_KEYUP:
                    for(Window*& w : Window::container) w->handleKeyUp(e.key); 
                    break;
                case SDL_TEXTINPUT:
                    for(Window*& w : Window::container) w->handleTextInput(e.text); 
                    break;
                case SDL_TEXTEDITING:
                    std::cout << e.edit.text << std::endl;
                    break;
                }
            }
        };
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(loop_func, NULL, fps, -1);
#else
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
    bool MainLoop::looping = true;

}
