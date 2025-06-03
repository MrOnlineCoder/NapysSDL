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

    SDL_Window *window = SDL_CreateWindow("Napys Test", 1024, 768, 0);
    if (window == NULL)
    {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    if (!TTF_Init())
    {
        fprintf(stderr, "Could not initialize SDL_ttf: %s\n", TTF_GetError());
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

        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

NapysRendererTTF *NapysCreateRendererTTF(NapysContext *ctx, TTF_TextEngine *engine)
{
    NapysRendererTTF *nrttf = SDL_malloc(sizeof(NapysRendererTTF));

    if (!nrttf)
    {
        NapysSetError("Failed to allocate memory for NapysRendererTTF");
        return NULL;
    }

    nrttf->ctx = ctx;
    nrttf->engine = engine;

    return nrttf;
}

void NapysDestroyRendererTTF(NapysRendererTTF *renderer)
{
    if (renderer)
    {
        SDL_free(renderer);
    }
}

void NapysExecuteCommandList(NapysRendererTTF *renderer, NapysCommandList *list)
{
    if (!renderer || !list)
    {
        NapysSetError("Invalid renderer or command list");
        return;
    }
}