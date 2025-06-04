#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <napys.h>

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Napys Test", 800, 600, 0);
    if (window == NULL)
    {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    if (!TTF_Init())
    {
        fprintf(stderr, "Could not initialize SDL_ttf: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL)
    {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    NapysContext *nsctx = NapysCreateContext();

    if (!nsctx)
    {
        fprintf(stderr, "Could not create Napys context\n");
        TTF_Quit();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("Roboto.ttf", 24);

    NapysRegisterFont(nsctx, font, "main");

    NapysRegisterCSSColors(nsctx);

    NapysRegisterSize(nsctx, "main", 32);
    NapysRegisterSize(nsctx, "small", 24);
    NapysRegisterSize(nsctx, "accent", 42);

    SDL_Surface *icon_surface = SDL_LoadBMP("icon.bmp");

    if (!icon_surface)
    {
        fprintf(stderr, "Could not load icon: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture *icon_texture = SDL_CreateTextureFromSurface(renderer, icon_surface);

    if (!icon_texture)
    {
        fprintf(stderr, "Could not create texture from surface: %s\n", SDL_GetError());
        return 1;
    }

    SDL_DestroySurface(icon_surface);

    NapysRegisterImage(nsctx, "icon", icon_texture);

    NapysCommandList *cmd_list = NapysParseRichText(
        "{{font:main}}{{size:main}}{{color:black}}Hello World from {{image:icon}}!{{color:red}} This will be red.{{newline}}{{color:green}}{{size:accent}}This will be green and big{{newline}}{{color:cyan}}Перевірка тексту українською"
        "{{newline}}{{color:silver}}{{size:small}}Smaller text here"
        "{{newline}}{{image:icon}}{{color:red}}R{{color:orange}}A{{color:yellow}}I{{color:green}}N{{color:blue}}B{{color:magenta}}O{{color:lightblue}}W{{image:icon}}",
        NULL);

    NapysRendererTTF *ns_renderer = NapysCreateRendererTTF(nsctx, renderer);

    NapysExecuteCommandList(ns_renderer, cmd_list);

    SDL_Rect local_bounds;

    NapysGetRenderedTextBounds(ns_renderer, &local_bounds);

    SDL_FRect bounds;
    bounds.x = 50 + local_bounds.x;
    bounds.y = 50 + local_bounds.y;
    bounds.w = local_bounds.w;
    bounds.h = local_bounds.h;

    bool running = true;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        NapysRenderTTF(ns_renderer, 50, 50);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderRect(renderer, &bounds);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    NapysDestroyCommandList(cmd_list);
    NapysDestroyRendererTTF(ns_renderer);
    NapysDestroyContext(nsctx);

    TTF_CloseFont(font);
    SDL_DestroyTexture(icon_texture);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
