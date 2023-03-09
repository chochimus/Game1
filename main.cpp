#include<SDL.h>
#include<SDL_image.h>
#include<iostream>

constexpr int SCREEN_WIDTH = 640;
constexpr int SCREEN_HEIGHT = 480;

//SDL Class begins here
class SDL {
public:
    SDL()
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error{ "SDL initialization failed: " + std::string{ SDL_GetError() } };
        }
        _own = true;
    }

    ~SDL()
    {
        if (_own) {
            SDL_Quit();
        }
    }

    // Non-copyable
    SDL(SDL const&) = delete;
    auto operator=(SDL const&)->SDL & = delete;

    // Move constructor and assignment
    SDL(SDL&& other) noexcept
    {
        _swap(*this, other);
    }

    auto operator=(SDL&& other) noexcept -> SDL&
    {
        _swap(*this, other);
        return *this;
    }

private:
    static void _swap(SDL& a, SDL& b) noexcept
    {
        std::swap(a._own, b._own);
    }

    bool _own = false;
};
//SDL class ends here
//SDL_Image class begins here
class IMG {
public:
    IMG() {
        if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
            throw std::runtime_error{ "SDL_image failed to initialize: " + std::string{SDL_GetError()} };
        }
        _own = true;
    }
    ~IMG() {
        if (_own) {
            IMG_Quit();
        }
    }
    //Non-copyable
    IMG(IMG const&) = delete;
    auto operator=(IMG const&)->IMG & = delete;
    //Move constructor and assignment
    IMG(IMG&& other) noexcept
    {
        _swap(*this, other);
    }
    auto operator=(IMG&& other) noexcept -> IMG& {
        _swap(*this, other);
        return *this;
    }

private:
    static void _swap(IMG& a, IMG& b) noexcept {
        std::swap(a._own, b._own);
    }
    bool _own = false;
};
// SDL_Image class ends here
//relevant deleters to simplify window/renderer creation
struct sdl_window_deleter
{
    void operator()(SDL_Window* p) const noexcept
    {
        if (p)
            SDL_DestroyWindow(p);
    }
};

struct sdl_renderer_deleter
{
    void operator()(SDL_Renderer* p) const noexcept
    {
        if (p)
            SDL_DestroyRenderer(p);
    }
};

//TODO: Texture class
class LTexture {
    static struct from_surface_tag {} from_surface;

    LTexture(from_surface_tag, SDL_Renderer* renderer, std::string const& path);
    //Text texture initializer to add if needed

    LTexture(LTexture&& other) noexcept {
        using std::swap;
        swap(*this, other);
    }
    ~LTexture();

    void render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);

    LTexture& operator=(LTexture&& other) noexcept {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    SDL_Texture* mTexture = nullptr;
    int mWidth = 0;
    int mHeight = 0;
    
    LTexture(LTexture const&) = delete;
    LTexture& operator=(LTexture const&) = delete;

    friend void swap(LTexture&, LTexture&) noexcept;
};
LTexture::LTexture(from_surface_tag, SDL_Renderer* renderer, std::string const& path) {
    auto surface = IMG_Load(path.c_str());
    if (!surface) {
        throw std::runtime_error{ "Failed to load image texture: " + path };
    }
    mTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!mTexture) {
        throw std::runtime_error{ "Failed to create texture from surface" };
    }
    mWidth = surface->w;
    mHeight = surface->h;
}
void LTexture::render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    SDL_Rect renderQuad = { x,y, mWidth, mHeight };
    if (clip != nullptr) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopyEx(renderer, mTexture, clip, &renderQuad, angle, center, flip);
}
LTexture::~LTexture() {
    if (mTexture) {
        SDL_DestroyTexture(mTexture);
    }
}
void swap(LTexture& a, LTexture& b) noexcept {
    using std::swap;
    swap(a.mTexture, b.mTexture);
    swap(a.mWidth, b.mWidth);
    swap(a.mHeight, b.mHeight);
}
//TODO: PlayerClass

//main program
int main(int argc, char* argv[]) {
    try {

        SDL sdl;
        IMG img;

        std::unique_ptr<SDL_Window, sdl_window_deleter> window(
            SDL_CreateWindow("Environment", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN),
            sdl_window_deleter()
        );

        std::unique_ptr<SDL_Renderer, sdl_renderer_deleter> renderer(
            SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
            sdl_renderer_deleter()
        );


        //Main loop flag
        bool quit = false;
        SDL_Event e;

        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }

            }
            SDL_SetRenderDrawColor(renderer.get(), 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderClear(renderer.get());
            SDL_RenderPresent(renderer.get());
        }

        return 0;
    }
    catch (std::exception const& x)
    {
        std::cerr << "Error: " << x.what() << '\n';
        return EXIT_FAILURE;
    }
}