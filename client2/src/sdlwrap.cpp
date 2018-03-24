#include "sdlwrap.hpp"
#include <stdio.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#else
#endif

namespace sdlw{
    template<typename ... Args>
    static void __error(const char* s, int l, const char* f, Args const & ... args){
        printf("%s:%d: error: ", s, l);
        printf(f, args...);
        printf("\n");
        fflush(stdout);
    }
#define error(...) __error(__FILE__, __LINE__, __VA_ARGS__)

    char* FileManager::base_path = SDL_GetBasePath();
    char* FileManager::pref_path = NULL;
    void FileManager::setPrefPath(const char* company_name, const char* app_name){
        pref_path = SDL_GetPrefPath(company_name, app_name);
    }
    const char* FileManager::getPrefPath(){
        return pref_path;
    }
    const char* FileManager::getBasePath(){
        if(!base_path) base_path = SDL_GetBasePath();
        return base_path;
    }
    int FileManager::save_pref(const char* relative_path, void* data, size_t size){ 
        std::string path = pref_path;
        path += relative_path;
        FILE* f = fopen(path.c_str(), "w");
        if(f == NULL){
            error("file open at '%s' failed", path.c_str()); 
            return -1;
        }
        if(size != fwrite(data, 1, size, f)){
            error("file write at '%s' failed", path.c_str()); 
            fclose(f);
            return -2;
        }
        fclose(f);
        return 0;
    }
    int FileManager::load_pref(const char* relative_path, void* data, size_t size){ 
        std::string path = pref_path;
        path += relative_path;
        FILE* f = fopen(path.c_str(), "w");
        if(f == NULL){
            error("file open at '%s' failed", path.c_str()); 
            return -1;
        }
        if(size != fread(data, 1, size, f)){
            error("file read at '%s' failed", path.c_str()); 
            return -2;
        }
        fclose(f);
        return 0;
    }

    
    #define DEFAULT_TTF_PATH "asset/font/ttf/D2Coding-Ver1.3-20171129.ttf"
    template<int size>
    TTF_Font*& Font::get(){
        static TTF_Font* font = TTF_OpenFont((std::string(FileManager::getBasePath()) + DEFAULT_TTF_PATH).c_str(), size);
        if(font == NULL) error("%s with '%s'", SDL_GetError(), DEFAULT_TTF_PATH);
        return font;
    }
    template<char... path, int size>
    TTF_Font*& Font::get(){
        static TTF_Font* font = TTF_OpenFont((std::string(FileManager::getBasePath()) + std::string(path...)).c_str(), size);
        if(font == NULL) error("%s with '%s'", SDL_GetError(), path...);
        return font;
    }

    Texture::Texture(SDL_Renderer* renderer, const char* path) {
        load(renderer, path);
    }
    Texture::Texture(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor) {
        load(renderer, text, font, textColor);
    }
    void Texture::load(SDL_Renderer* renderer, const char* path){
        if(texture) free();
        SDL_Surface* loadedSurface = IMG_Load(path);
        if(loadedSurface == NULL) error("%s with '%s'", IMG_GetError(), path);
        //SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if(texture == NULL) error("%s with '%s'", SDL_GetError(), path);
        w = loadedSurface->w;
        h = loadedSurface->h;
        SDL_FreeSurface( loadedSurface );
    }
    void Texture::load(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor){
        if(texture) free();
        SDL_Surface* loadedSurface = TTF_RenderUTF8_Blended(font, text, textColor);
        if(loadedSurface == NULL) error("%s with '%s'", TTF_GetError(), text);
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if(texture == NULL) error("texture of '%s' load fail: %s", text, SDL_GetError());
        w = loadedSurface->w;
        h = loadedSurface->h;
        SDL_FreeSurface(loadedSurface);
    }
    void Texture::free(){ SDL_DestroyTexture(texture); }
    Texture::~Texture(){ 
        if(texture) free(); 
    }
    /*
    Texture& Texture::operator=(const Texture& fellow)
    {
        if(texture) SDL_DestroyTexture(texture);
        return *this;
    }
    */
    //void setColor( Uint8 red, Uint8 green, Uint8 blue ){}
    //void setBlendMode( SDL_BlendMode blending ){}
    //void setAlpha( Uint8 alpha ){}
    //void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE ){}

    SpriteSheet::SpriteSheet(SDL_Renderer* renderer, const char* atlas_info_csv) {
        load(renderer, atlas_info_csv);
    } 
    int SpriteSheet::load(SDL_Renderer* renderer, const char* atlas_info_csv){
        int field_num = 11;
        char *saveptr=NULL;
        char *atlas_name=NULL, *sprite_name=NULL;
        bool rotated;
        SDL_Rect source_rect;
        FILE* f = fopen(atlas_info_csv, "r");
        if(f == NULL){
            error("can not open file '%s'",atlas_info_csv);
            return -1;
        }
        fseek(f, 0, SEEK_END);
        int l = ftell(f);
        rewind(f);
        char* str = new char[l];
        if(l != fread(str, 1, l, f)){
            error("error while reading '%s'", atlas_info_csv);
            return -1;
        }
        fclose(f);
        strtok_r(str, ", \n", &saveptr);
        --field_num;
        while(true){
            for(;field_num; --field_num) strtok_r(NULL, ", \n", &saveptr);
            if(!(atlas_name = strtok_r(NULL, ", \n", &saveptr))) break;
            if(path2atlas.find(atlas_name) == path2atlas.end())
                path2atlas.insert({atlas_name, new Texture(renderer, atlas_name)});
            //if(path2atlas.find(std::string(atlas_name)) == path2atlas.end())
            //    atlas_texture = &path2atlas.insert({std::string(atlas_name), Texture(renderer, atlas_name)}).first->second;
            //atlas_texture = &path2atlas.at(std::string(atlas_name));
            for(int i=4;i;--i) strtok_r(NULL, ", \n", &saveptr);
            sprite_name = strtok_r(NULL, ", \n", &saveptr);
            rotated = atoi(strtok_r(NULL, ", \n", &saveptr));
            source_rect.w = atoi(strtok_r(NULL, ", \n", &saveptr));
            source_rect.h = atoi(strtok_r(NULL, ", \n", &saveptr)); 
            source_rect.x = atoi(strtok_r(NULL, ", \n", &saveptr));
            source_rect.y = atoi(strtok_r(NULL, ", \n", &saveptr)); 
            if(path2sprite.find(atlas_name) == path2sprite.end())
                path2sprite.insert({sprite_name, new Sprite(path2atlas.at(atlas_name), source_rect, rotated)});
            else
                error("there are duplicated sprites: '%s'", sprite_name);
        }
        if(sprite_name == NULL || sprite_name[0] == '\0'){
            error("error while loading atlas at '%s'", atlas_info_csv);
            return -1;
        }
        delete[] str;
        return 0;
    }

    void Window::initSDL() {
        // it will be initialized when first window created
        // * DO NOT MANUALLY CALL *
        static bool not_yet = true;
        if(not_yet){
            not_yet = false;
            if(SDL_Init(SDL_INIT_VIDEO) < 0) error("fail while init SDL: %s", SDL_GetError());
            if(TTF_Init() < 0) error("fail while init TTF: %s", TTF_GetError());
            if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) error("%s", IMG_GetError());
        }
    }
    void Window::render(){
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        if(current_scene) current_scene->render();
        SDL_RenderPresent(renderer);
    }
    Window::Window(MainLoop* mainloop, const char* title, int w, int h): Window(title, w, h) {
        mainloop->add(this);
    }
    Window::Window(const char* title, int w, int h): Window(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h) {
    }
    Window::Window(MainLoop* mainloop, const char* title, int x, int y, int w, int h, uint32_t flags): Window(title, x, y, w, h, flags) {
        mainloop->add(this);
    }
    Window::Window(const char* title, int x, int y, int w, int h, uint32_t flags): width(w), height(h){
        initSDL();
        if(!(window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags))) error("fail while gen window '%s': %s", title, SDL_GetError());
        if(!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED))) error("error while gen renderer of window of '%s': %s", title, SDL_GetError());
        if(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) error("error while set render draw blend mode with window of '%s'", title, SDL_GetError());
    }
    Window::~Window(){
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
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

    Component::Component(Scene* scene): renderer(scene->getRenderer()){
        scene->add(this);
    }
    SDL_Renderer* Component::getRenderer(){ return renderer; }

    void Scene::render(){ 
        for(auto& c: components) c->render();
    }
    Scene::Scene(Window* w){
        window = w; 
        if(!window->getCurrentScene()) window->set(this);
    }
    bool Scene::operator==(const Scene& rhs) const {
        return (this==&rhs);
    }
    void Scene::show(){ 
        window->set(this); 
    }
    void Scene::switchTo(Scene* other){ other->show(); }
    void Scene::handleMouseMotion(const SDL_MouseButtonEvent& e){
        if(mouseMotionHandler) mouseMotionHandler(e);
        for(auto& c: components)
            c->handleMouseMotion(e);
    }
    void Scene::handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(mouseButtonDownHandler) mouseButtonDownHandler(e);
        for(auto& c: components)
            c->handleMouseButtonDown(e);
    }
    void Scene::handleMouseButtonUp(const SDL_MouseButtonEvent& e){
        if(mouseButtonUpHandler) mouseButtonUpHandler(e);
        for(auto& c: components)
            c->handleMouseButtonUp(e);
    }
    void Scene::handleTextInput(const SDL_TextInputEvent& e){
        if(textInputHandler) textInputHandler(e);
        for(auto& c: components)
            c->handleTextInput(e);
    }
    void Scene::handleKeyDown(const SDL_KeyboardEvent& e){
        if(keyDownHandler) keyDownHandler(e);
        for(auto& c: components)
            c->handleKeyDown(e);
    }
    void Scene::handleKeyUp(const SDL_KeyboardEvent& e){
        if(keyUpHandler) keyUpHandler(e);
        for(auto& c: components)
            c->handleKeyUp(e);
    }
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

    Text::Text(Scene* scene, int x, int y, const char* str, TTF_Font* _font, SDL_Color _color, Align align_v, Align align_h): Component(scene), align_v(align_v), align_h(align_h) {
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
            texture.load(renderer, text.c_str(), font, color);
            rect.w = texture.getWidth(); 
            rect.h = texture.getHeight();
        }
        else{
            //texture.free();
            rect.w = 0;
        }
    }
    void Text::render(){
        texture.render(renderer, rect);
    }

    Input::Input(Scene* scene, int x, int y, int w, TTF_Font* _font, SDL_Color _color, Align align_v, Align align_h, char overlap): Text::Text(scene, x, y, NULL, _font, _color, align_v, Align::LEFT), bg_align_h(align_h), overlap(overlap) {
        cursor_poses.push_back(0);
        bg_rect = {x, y, w, TTF_FontHeight(_font)+3};
        cursor_rect = {x+rect.w, y, 1, TTF_FontHeight(_font)};
        underbar_rect = {x, y+bg_rect.h-1, w, 1};
        set(x, y);
    }
    void Input::focusIn(){ 
#if defined(__EMSCRIPTEN__)
        EM_ASM(
            Module.openOnScreenKeyboard();
        );
#endif
        focused = this; 
    };
    void Input::focusOut(){ 
#if defined(__EMSCRIPTEN__)
        EM_ASM(
            Module.shutOnScreenKeyboard();
        );
#endif
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
        else if(focused == this){
            focusOut();
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
        texture.render(renderer, rect);
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
        texture.render(renderer, rect);
    }

    void MainLoop::run(int fps){
        SDL_StartTextInput();
        auto loop_func = [](void* args){
            MainLoop* _this = reinterpret_cast<MainLoop*>(args);
            while(SDL_PollEvent(&_this->e)) {
                switch(_this->e.type){
                case SDL_QUIT: 
                    _this->MainLoop::stop();
                    break;
                case SDL_MOUSEMOTION:
                    for(Window*& w : _this->windows) w->handleMouseMotion(_this->e.button); 
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    for(Window*& w : _this->windows) w->handleMouseButtonDown(_this->e.button); 
                    break;
                case SDL_MOUSEBUTTONUP:
                    for(Window*& w : _this->windows) w->handleMouseButtonUp(_this->e.button); 
                    break;
                case SDL_KEYDOWN:
                    for(Window*& w : _this->windows) w->handleKeyDown(_this->e.key); 
                    break;
                case SDL_KEYUP:
                    for(Window*& w : _this->windows) w->handleKeyUp(_this->e.key); 
                    break;
                case SDL_TEXTINPUT:
                    for(Window*& w : _this->windows) w->handleTextInput(_this->e.text); 
                    break;
                case SDL_TEXTEDITING:
                    printf("%s\n", _this->e.edit.text);
                    break;
                case SDL_FINGERDOWN:
                    for(Window*& w : _this->windows){ 
                        _this->fake_mve.x = _this->e.tfinger.x * w->getWidth();
                        _this->fake_mve.y = _this->e.tfinger.y * w->getHeight();
                        w->handleMouseButtonDown(_this->fake_mve); 
                    }
                    break;
                case SDL_FINGERUP:
                    for(Window*& w : _this->windows){ 
                        _this->fake_mve.x = _this->e.tfinger.x * w->getWidth();
                        _this->fake_mve.y = _this->e.tfinger.y * w->getHeight();
                        w->handleMouseButtonUp(_this->fake_mve); 
                    }
                    break;
                case SDL_FINGERMOTION:
                    for(Window*& w : _this->windows){ 
                        _this->fake_mve.x = _this->e.tfinger.x * w->getWidth();
                        _this->fake_mve.y = _this->e.tfinger.y * w->getHeight();
                        w->handleMouseMotion(_this->fake_mve); 
                    }
                    break;
                }
            }
            while(SDL_GetTicks() - _this->last_time >= 1000/60){
                for(Window*& w : _this->windows) w->render(); 
                _this->last_time += 1000/60;
            }
        };
        last_time = SDL_GetTicks();
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(loop_func, this, fps, true);
#else
        uint32_t last_time = SDL_GetTicks(); 
        while(looping){
            loop_func(this);
            if(SDL_GetTicks() - last_time < 1000/fps)
                SDL_Delay(1000/fps - SDL_GetTicks() + last_time);
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
    void MainLoop::add(Window* window){
        windows.push_back(window);
    }
}
