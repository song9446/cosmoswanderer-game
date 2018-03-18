#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <fstream>
namespace sdlwrap{

struct UserDataSaver {
    static const char* user_directory_path;
    static int save(const char* relative_path, void* data, size_t size); 
    static int load(const char* relative_path, void* data, size_t size); 
};

struct Font{
    static constexpr const char* DEFAULT_TTF_PATH = "asset/font/ttf/NanumBarunGothic.ttf";
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
    SDL_Rect rect;
    SDL_Renderer* renderer = NULL;
    int w, h;
public:
    Texture(SDL_Renderer* renderer);
    Texture(SDL_Renderer* renderer, const char* path);
    Texture(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor = SDL_Color{255,255,255,255});
    void load(const char* path);
    void load(const char* text, TTF_Font* font, SDL_Color textColor = SDL_Color{255,255,255, 255});
    void free();
    ~Texture();
    Texture& operator=(const Texture& fellow);
    //void setColor( Uint8 red, Uint8 green, Uint8 blue ){}
    //void setBlendMode( SDL_BlendMode blending ){}
    //void setAlpha( Uint8 alpha ){}
    //void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE ){}
    inline void render(const SDL_Rect& rect){
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }
    inline void render(int x, int y){ 
        rect.x = x; rect.y = y; rect.w = w; rect.h = h;
        SDL_RenderCopy(renderer, texture, NULL, &rect); 
    }
    int getWidth(){return w;};
    int getHeight(){return h;};
};

class EventHandlerInterface { 
    typedef std::function<void (const SDL_MouseButtonEvent&)> SDLMouseButtonEventHandler;
protected:
    SDLMouseButtonEventHandler mouseMotionHandler = {};
    SDLMouseButtonEventHandler mouseButtonDownHandler = {};
    SDLMouseButtonEventHandler mouseButtonUpHandler = {};
public:
    void listenMouseMotion(SDLMouseButtonEventHandler&& h){ mouseMotionHandler = std::move(h); };
    void listenMouseDown(SDLMouseButtonEventHandler&& h){ mouseButtonDownHandler = std::move(h); };
    void listenMouseUp(SDLMouseButtonEventHandler&& h){ mouseButtonUpHandler = std::move(h); };
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e) = 0;
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e) = 0;
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e) = 0;
};

class Scene;
class Window: public EventHandlerInterface {
    SDL_Window* window;
    SDL_Renderer* renderer;
    Scene* current_scene;
    int width, height;
    static void initSDL(); 
    static std::vector<Window*> container;
    void render();
public:
    Window(const char* title, int w, int h);
    Window(const char* title, int x, int y, int w, int h, uint32_t flags=SDL_WINDOW_SHOWN);
    ~Window();
    void set(Scene* scene); 
    void set(Scene& scene); 
    SDL_Renderer* getRenderer();
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e);
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e);
    friend class MainLoop;
};

class Component: public EventHandlerInterface {
protected:
    SDL_Renderer* renderer;
    SDL_Renderer* getRenderer();
    SDL_Rect rect;
public:
    virtual void render() = 0;
    Component(Scene& scene);
    void handleMouseMotion(const SDL_MouseButtonEvent& e){
        if(mouseMotionHandler && isCollision(rect, e.x, e.y))
            mouseMotionHandler(e);
    }
    void handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(mouseButtonDownHandler && isCollision(rect, e.x, e.y))
            mouseButtonDownHandler(e);
    }
    void handleMouseButtonUp(const SDL_MouseButtonEvent& e){
        if(mouseButtonUpHandler && isCollision(rect, e.x, e.y))
            mouseButtonUpHandler(e);
    }
};

class Scene: public EventHandlerInterface {
    typedef const std::function<void (char*, int)> Collable;
    Window* window;
    std::vector<Component*> components;
    void render();
public:
    Scene(Window& w);
    void show();
    void switchTo(Scene& other);
    void handleMouseMotion(const SDL_MouseButtonEvent& e){
        if(mouseMotionHandler) mouseMotionHandler(e);
        for(auto& c: components)
            c->handleMouseMotion(e);
    }
    void handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(mouseButtonDownHandler) mouseButtonDownHandler(e);
        for(auto& c: components)
            c->handleMouseButtonDown(e);
    }
    void handleMouseButtonUp(const SDL_MouseButtonEvent& e){
        if(mouseButtonUpHandler) mouseButtonUpHandler(e);
        for(auto& c: components)
            c->handleMouseButtonUp(e);
    }
    void add(Component& c);

    bool operator==(const Scene& rhs) const;

    friend class Window;
    friend Component::Component(Scene&);
};

class Text: public Component {
    Texture texture;
    std::string text;
    TTF_Font* font = Font::get<16>();
    SDL_Color color = SDL_Color{255,255,255,255};
public:
    Text(Scene& scene, int x, int y, const char* str);
    Text(Scene& scene, int x, int y, const char* str, TTF_Font* _font, SDL_Color _color);
    void set(int _x, int _y);
    void set(const char* str, TTF_Font* _font, SDL_Color _color);
    void set(const char* str);
    void render();
};

class MainLoop {
    static bool looping;
public:
    static void run(int fps=-1);
    static void stop();
};
}
