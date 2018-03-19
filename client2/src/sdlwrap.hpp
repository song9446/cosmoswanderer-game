#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <fstream>
namespace sdlwrap{
enum class Align {
    CENTER, LEFT, RIGHT, TOP, MIDDLE, BOTTOM
};

struct UserDataSaver {
    static const char* user_directory_path;
    static int save(const char* relative_path, void* data, size_t size); 
    static int load(const char* relative_path, void* data, size_t size); 
};

struct Font{
    static constexpr const char* DEFAULT_TTF_PATH = "asset/font/ttf/D2Coding-Ver1.3-20171129.ttf";
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
    Align align_v, align_h;
    int w, h;
public:
    Texture(SDL_Renderer* renderer, Align _align_v=Align::TOP, Align _align_h=Align::LEFT);
    Texture(SDL_Renderer* renderer, const char* path, Align _align_v=Align::TOP, Align _align_h=Align::LEFT);
    Texture(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color textColor = SDL_Color{255,255,255,255}, Align _align_v=Align::TOP, Align _align_h=Align::LEFT);
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
        rect.x = x - (align_h==Align::CENTER)*w/2 - (align_h==Align::RIGHT)*w; 
        rect.y = y - (align_v==Align::MIDDLE)*h/2 - (align_v==Align::BOTTOM)*h; 
        rect.w = w; rect.h = h;
        SDL_RenderCopy(renderer, texture, NULL, &rect); 
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
class Window: public EventHandlerInterface {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    Scene* current_scene = NULL;
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
    friend class Scene;
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

class Scene: public EventHandlerInterface {
    typedef const std::function<void (char*, int)> Collable;
    Window* window;
    std::vector<Component*> components;
    void render();
public:
    Scene(Window& w);
    ~Scene();
    void show();
    void switchTo(Scene* other);
    virtual void handleMouseMotion(const SDL_MouseButtonEvent& e){
        if(mouseMotionHandler) mouseMotionHandler(e);
        for(auto& c: components)
            c->handleMouseMotion(e);
    }
    virtual void handleMouseButtonDown(const SDL_MouseButtonEvent& e){
        if(mouseButtonDownHandler) mouseButtonDownHandler(e);
        for(auto& c: components)
            c->handleMouseButtonDown(e);
    }
    virtual void handleMouseButtonUp(const SDL_MouseButtonEvent& e){
        if(mouseButtonUpHandler) mouseButtonUpHandler(e);
        for(auto& c: components)
            c->handleMouseButtonUp(e);
    }
    virtual void handleTextInput(const SDL_TextInputEvent& e){
        if(textInputHandler) textInputHandler(e);
        for(auto& c: components)
            c->handleTextInput(e);
    }
    virtual void handleKeyDown(const SDL_KeyboardEvent& e){
        if(keyDownHandler) keyDownHandler(e);
        for(auto& c: components)
            c->handleKeyDown(e);
    }
    virtual void handleKeyUp(const SDL_KeyboardEvent& e){
        if(keyUpHandler) keyUpHandler(e);
        for(auto& c: components)
            c->handleKeyUp(e);
    }
    void add(Component* c);
    template<class ComponentType, typename... Args>
    ComponentType* add(Args... args);

    bool operator==(const Scene& rhs) const;

    friend class Window;
    friend Component::Component(Scene*);
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
    static bool looping;
public:
    static void run(int fps=-1);
    static void stop();
};
}
