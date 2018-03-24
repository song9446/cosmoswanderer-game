#define SDL_MAIN_HANDLED 1
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <math.h>
namespace sdlw{
enum class Align {
    CENTER, LEFT, RIGHT, TOP, MIDDLE, BOTTOM
};

struct FileManager {
    static char* base_path;
    static char* pref_path;
    static void setPrefPath(const char* company_name, const char* app_name);
    static const char* getPrefPath();
    static const char* getBasePath();
    static int save_pref(const char* relative_path, void* data, size_t size); 
    static int load_pref(const char* relative_path, void* data, size_t size); 
};

struct Font{
    //static const char* DEFAULT_TTF_PATH;
    template<char... path, int size>
    static TTF_Font*& get();
    template<int size>
    static TTF_Font*& get();
};
static inline bool isCollision(const SDL_Rect rect, const SDL_Point point){
    return (rect.x < point.x) && (rect.x+rect.w > point.x)
        && (rect.y < point.y) && (rect.y+rect.h > point.y);
}
static inline bool isCollision(const SDL_Rect rect, const int x, const int y){
    return (rect.x < x) && (rect.x+rect.w > x)
        && (rect.y < y) && (rect.y+rect.h > y);
}
/*
static inline iscollision(const SDL_Rect r1, const SDL_Rect r2){
    return (rect.x < point.x) && (rect.x+rect.w > point.x)
        && (rect.y < point.y) && (rect.y+rect.h > point.y);
}
*/

class Texture{
    SDL_Texture* texture = NULL;
    int w, h;
public:
    Texture(){}
    Texture(SDL_Renderer* renderer, Texture* other);
    Texture(SDL_Renderer* renderer, const char* path);
    Texture(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor = SDL_Color{255,255,255,255});
    void load(SDL_Renderer* renderer, const char* path);
    void load(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor = SDL_Color{255,255,255, 255});
    void free();
    ~Texture();
    //Texture& operator=(const Texture& fellow);
    //void setColor( Uint8 red, Uint8 green, Uint8 blue ){}
    //void setBlendMode( SDL_BlendMode blending ){}
    //void setAlpha( Uint8 alpha ){}
    //void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE ){}
    inline void render(SDL_Renderer* renderer, const SDL_Rect& dest_rect){
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
    }
    inline void render(SDL_Renderer* renderer, const SDL_Rect& source_rect, const SDL_Rect& dest_rect, const double angle, const SDL_Point& pivot, const SDL_RendererFlip flip = SDL_FLIP_NONE){
        SDL_RenderCopyEx(renderer, texture, &source_rect, &dest_rect, angle, &pivot, flip);
    }
    inline void render(SDL_Renderer* renderer, const SDL_Rect& source_rect, const SDL_Rect& dest_rect, SDL_RendererFlip flip = SDL_FLIP_NONE){
        SDL_RenderCopyEx(renderer, texture, &source_rect, &dest_rect, 0. , NULL, flip);
    }
    inline void render(SDL_Renderer* renderer, int x, int y, Align align_v = Align::TOP, Align align_h = Align::LEFT){ 
        SDL_Rect dest_rect{x, y, w, h};
        dest_rect.x = x - (align_h==Align::CENTER)*dest_rect.w/2 - (align_h==Align::RIGHT)*(dest_rect.w); 
        dest_rect.y = y - (align_v==Align::MIDDLE)*dest_rect.h/2 - (align_v==Align::BOTTOM)*(dest_rect.h); 
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect); 
    }
    int getWidth(){return w;};
    int getHeight(){return h;};
};


class EventHandlerInterface { 
    typedef std::function<void (const SDL_MouseButtonEvent&)> SDLMouseButtonEventHandler;
    typedef std::function<void (const SDL_TextInputEvent&)> SDLTextInputEventHandler;
    typedef std::function<void (const SDL_KeyboardEvent&)> SDLKeyboardEventHandler;
protected:
    SDLMouseButtonEventHandler mouseMotionHandler = {};
    SDLMouseButtonEventHandler mouseButtonDownHandler = {};
    SDLMouseButtonEventHandler mouseButtonUpHandler = {};
    SDLTextInputEventHandler textInputHandler = {};
    SDLKeyboardEventHandler keyDownHandler = {};
    SDLKeyboardEventHandler keyUpHandler = {};
public:
    void listenMouseMotion(SDLMouseButtonEventHandler&& h){ mouseMotionHandler = std::move(h); };
    void listenMouseDown(SDLMouseButtonEventHandler&& h){ mouseButtonDownHandler = std::move(h); };
    void listenMouseUp(SDLMouseButtonEventHandler&& h){ mouseButtonUpHandler = std::move(h); };
    void listenTextInput(SDLTextInputEventHandler&& h){ textInputHandler = std::move(h); };
    void listenKeyDown(SDLKeyboardEventHandler&& h){ keyDownHandler = std::move(h); };
    void listenKeyUp(SDLKeyboardEventHandler&& h){ keyUpHandler = std::move(h); };
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e) = 0;
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e) = 0;
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e) = 0;
    virtual void handleTextInput(const SDL_TextInputEvent& e) = 0;
    virtual void handleKeyDown(const SDL_KeyboardEvent& e) = 0;
    virtual void handleKeyUp(const SDL_KeyboardEvent& e) = 0;
};

class Scene;
class MainLoop;
class Window: public EventHandlerInterface {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    Scene* current_scene = NULL;
    int width, height;
    static void initSDL(); 
    void render();
public:
    Window(const char* title, int w, int h);
    Window(const char* title, int x, int y, int w, int h, uint32_t flags=SDL_WINDOW_SHOWN);
    Window(MainLoop* mainloop, const char* title, int w, int h);
    Window(MainLoop* mainloop, const char* title, int x, int y, int w, int h, uint32_t flags=SDL_WINDOW_SHOWN);
    ~Window();
    Scene* getCurrentScene(){ return current_scene; }
    void set(Scene* scene); 
    void set(Scene& scene); 
    int getWidth(){ return width; }
    int getHeight(){ return height; }
    SDL_Renderer* getRenderer();
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e);
    virtual void handleTextInput(const SDL_TextInputEvent& e);
    virtual void handleKeyDown(const SDL_KeyboardEvent& e);
    virtual void handleKeyUp(const SDL_KeyboardEvent& e);
    friend class MainLoop;
    //friend class Scene;
};

class Component;
class Scene: public EventHandlerInterface {
    typedef const std::function<void (char*, int)> Collable;
    Window* window;
    std::vector<Component*> components;
    void render();
public:
    Scene(Window* w);
    ~Scene();
    SDL_Renderer* getRenderer(){
        return window->getRenderer();
    };
    void show();
    void switchTo(Scene* other);
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e);
    virtual void handleTextInput(const SDL_TextInputEvent& e);
    virtual void handleKeyDown(const SDL_KeyboardEvent& e);
    virtual void handleKeyUp(const SDL_KeyboardEvent& e);
    void add(Component* c);
    template<class ComponentType, typename... Args>
    ComponentType* add(Args... args);
    bool operator==(const Scene& rhs) const;
    friend class Window;
};

class Component: public EventHandlerInterface {
protected:
    SDL_Renderer* renderer;
    SDL_Renderer* getRenderer();
    SDL_Rect rect;
public:
    virtual void render() = 0;
    Component(Scene* scene);
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e){
        if(mouseMotionHandler && isCollision(rect, e.x, e.y))
            mouseMotionHandler(e);
    }
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(mouseButtonDownHandler && isCollision(rect, e.x, e.y))
            mouseButtonDownHandler(e);
    }
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e){
        if(mouseButtonUpHandler && isCollision(rect, e.x, e.y))
            mouseButtonUpHandler(e);
    }
    virtual void handleTextInput(const SDL_TextInputEvent& e){
        if(textInputHandler)
            textInputHandler(e);
    }
    virtual void handleKeyDown(const SDL_KeyboardEvent& e){
        if(keyDownHandler)
            keyDownHandler(e);
    }
    virtual void handleKeyUp(const SDL_KeyboardEvent& e){
        if(keyUpHandler)
            keyUpHandler(e);
    }
};

class Sprite {
    Texture* texture;
    SDL_Rect source_rect;
    bool rotated;
public:
    Sprite(Sprite* sprite) {
        texture = sprite->texture;
        source_rect = sprite->source_rect;
        rotated = sprite->rotated;
    }
    Sprite(Texture* texture, SDL_Rect source_rect, bool rotated): texture(texture), source_rect(source_rect), rotated(rotated) {}
    inline void render(SDL_Renderer* renderer, const SDL_Rect& dest_rect){
        texture->render(renderer, source_rect, dest_rect);
    }
    inline void render(SDL_Renderer* renderer, int x, int y, Align align_v = Align::TOP, Align align_h = Align::LEFT){ 
        SDL_Rect dest_rect = {x, y, source_rect.w, source_rect.h};
        SDL_Point pivot = {source_rect.w/2, source_rect.w/2};
        dest_rect.x = x - (align_h==Align::CENTER)*dest_rect.h/2 - (align_h==Align::RIGHT)*(dest_rect.h); 
        dest_rect.y = y - (align_v==Align::MIDDLE)*dest_rect.w/2 - (align_v==Align::BOTTOM)*(dest_rect.w); 
        texture->render(renderer, source_rect, dest_rect, -(rotated? 90.:0), pivot, SDL_FLIP_NONE);
    }
    inline void render(SDL_Renderer* renderer, int x, int y, const double angle_, const SDL_Point& p, SDL_RendererFlip flip = SDL_FLIP_NONE, Align align_v=Align::TOP, Align align_h=Align::LEFT){
        render(renderer, x, y, angle_, &p, flip, align_v, align_h);
    }
    inline void render(SDL_Renderer* renderer, int x, int y, const double angle_, const SDL_Point* _p, SDL_RendererFlip flip = SDL_FLIP_NONE, Align align_v=Align::TOP, Align align_h=Align::LEFT){
        float rad = 3.14159268357/180*angle_, 
              cosa = cos(-rad), sina = sin(-rad);
        SDL_Point t = {source_rect.w/2, source_rect.w/2},
                  p = _p==NULL? SDL_Point{source_rect.w/2, source_rect.h/2} : *_p;
        SDL_Rect dest_rect = {x, y, source_rect.w, source_rect.h};
        dest_rect.x = x - (align_h==Align::CENTER)*dest_rect.h/2 - (align_h==Align::RIGHT)*(dest_rect.h) - (p.x + cosa*(-p.x+t.x+t.y) + sina*(p.y+t.x-t.y)); 
        dest_rect.y = y - (align_v==Align::MIDDLE)*dest_rect.w/2 - (align_v==Align::BOTTOM)*(dest_rect.w) - (p.y - cosa*(-p.y-t.x+t.y) + sina*(-p.x+t.x+t.y)); 
        texture->render(renderer, source_rect, dest_rect, angle_ - (rotated? 90.:0), {0,0}, flip);
    }
};

/*
class AnimatedSprite {
    Sprite* sprites;
    uint32_t loop_num;
    uint32_t last_update_time;
    float speedfactor=1.0;
    AnimatedSprite(){
        sprites = NULL;
    }
    animate(){
    }
};
*/

class SpriteSheet {
    std::map<std::string, Texture*> path2atlas;
    std::map<std::string, Sprite*> path2sprite;
public:
    SpriteSheet(SDL_Renderer* renderer, const char* atlas_info_csv); 
    int load(SDL_Renderer* renderer, const char* atlas_info_csv);
    Sprite* get(const char* path){
        return path2sprite.at(std::string(path));
    }
};

class Image: public Component {
    Texture texture;
    int x, y;
public:
    Image(Scene* scene, const char* path, int x, int y): Component(scene), texture(renderer, path), x(x), y(y) {
    }
    void render(){
        texture.render(renderer, x ,y);
    }
};
class SpriteImage: public Component {
    Sprite sprite;
    int x, y;
    double angle;
    SDL_Point* pivot;
    Align align_v, align_h;
public:
    SpriteImage(Scene* scene, Sprite* sprite, int x, int y, Align align_v=Align::TOP, Align align_h=Align::LEFT): Component(scene), sprite(sprite), x(x), y(y), align_v(align_v), align_h(align_h){
    }
    SpriteImage(Scene* scene, Sprite* sprite, int x, int y, double angle, SDL_Point* pivot, Align align_v=Align::TOP, Align align_h=Align::LEFT): Component(scene), sprite(sprite), x(x), y(y), angle(angle), pivot(pivot? new SDL_Point{pivot->x, pivot->y} : NULL), align_v(align_v), align_h(align_h) {
    }
    ~SpriteImage(){
        delete pivot;
    }
    void render(){
        sprite.render(renderer, x, y, angle, pivot, SDL_FLIP_NONE, align_v, align_h);
    }
};

class Text: public Component {
protected:
    Texture texture;
    std::string text;
    TTF_Font* font = Font::get<16>();
    SDL_Color color = SDL_Color{255,255,255,255};
    Align align_v, align_h;
public:
    Text(Scene* scene, int x, int y, const char* str, TTF_Font* _font = Font::get<16>(), SDL_Color _color = SDL_Color{255, 255 ,255, 255}, Align align_v=Align::TOP, Align align_h=Align::LEFT);
    void set(int _x, int _y);
    void set(const char* str, TTF_Font* _font, SDL_Color _color);
    void set(const char* str);
    void render();
};

class Input: public Text {
protected:
    std::vector<int> cursor_poses;
    static Input* focused;
    int cursor=0;
    int counter=0; // used in bar blink. assume rendering fps is 60.
    SDL_Rect bg_rect, cursor_rect, underbar_rect;
    Align bg_align_h;
    char overlap = '\0';
    std::string real_text;
public:
    Input(Scene* scene, int x, int y, int w, TTF_Font* _font = Font::get<16>(), SDL_Color _color = SDL_Color{255, 255, 255 ,255}, Align align_v=Align::TOP, Align align_h=Align::LEFT, char overlap='\0');
    void set(int _x, int _y);
    void set(const char* str, TTF_Font* _font, SDL_Color _color);
    void set(const char* str);
    void setCursor(int index);
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e);
    virtual void handleTextInput(const SDL_TextInputEvent& e);
    virtual void handleKeyDown(const SDL_KeyboardEvent& e);
    void focusIn();
    void focusOut();
    void render();
};

class Button: public Text {
    SDL_Rect bg_rect;
    Align bg_align_h, bg_align_v;
    SDL_Color bg_color = SDL_Color{255, 255, 255, 0};
    bool down = false;
public:
    Button(Scene* scene, int x, int y, int w, int h, const char* str, TTF_Font* _font = Font::get<16>(), SDL_Color _color = SDL_Color{255, 255, 255 ,255}, Align align_v=Align::TOP, Align align_h=Align::LEFT);
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e);
    void render();
    void set(int _x, int _y);
};

class MainLoop {
#ifndef __EMSCRIPTEN__
    bool looping = true;
#endif
    SDL_Event e;
    SDL_MouseButtonEvent fake_mve; 
    Uint32 last_time;
    std::vector<Window*> windows;
public:
    void add(Window* window);
    void run(int fps=-1);
    void stop();
};
}
