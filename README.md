# Napys SDL

**Napys** (from Ukrainian _напис_, meaning "label") is a SDL3 / SDL_ttf extension library for rendering formatted rich-texts, written in C.

## Installation

Dependencies:

- SDL3
- SDL_ttf

You can link against CMake target `Napys` to include the library in your project.

## Features

- Provides API for rendering with `SDL_Renderer` and `TTF_TextEngine`
- Low-level command-based API for building rich texts.
- High-level API for parsing and rendering templates.
- Supports changing mid-text: color, font, size
- Supports drawing inline images

## Getting Started

TLDR: full example is available in the [examples/main.c](examples/main.c) file.

There are 3 main objects in Napys:

- **Context** (`NapysContext`)
- **Command List** (`NapysCommandList`)
- **Renderer** (`NapysRendererTTF`)

Napys works in some sense like LaTeX, where the output is built by executing a sequence of commands. A command may draw a text, change current font, color, size, draw an image, etc. **Command Lists** are used to store these commands.

A **Command List** are agnostic of any implementation or rendering details, and therefore, they shall be processed by a **Renderer**, which actually builds the expected output. Currently, Napys provides one renderer, `NapysRendererTTF`, which uses `TTF_TextEngine` under the hood to arrange multiple `TTF_Text` objects into single rich text, that can be drawn with `SDL_Renderer`.

Finally, to make things simpler, commands cannot use arbitrary values for font, color, style or image, all of that must be pre-defined in a **Context**, which acts like a registry of all available resources. The context is used to create command lists and renderers.
In a game application, you may create a context for each scenario of your text usage, one context for tutorial hints, one for dialogue, another one for UI, etc, as each label in given scenario will likely use the same set of fonts, colors and images.

That said, the typical setup for Napys looks as follows:

```c
//1. Create a context
NapysContext *ctx = NapysCreateContext();

// 2. Register any desired resources. 
// At least 1 font is required for rendering
TTF_Font *font = TTF_OpenFont(/*.... */);

NapysRegisterFont(ctx, font, "main"); //"main" here is an arbitrary internal name for the font.

NapysRegisterSize(ctx, "title", 32);
NapysRegisterSize(ctx, "description", 24);

NapysRegisterCSSColors(ctx); //register some common colors like white, black, yellow, etc...

NapysRegisterImage(ctx, "icon", icon_texture); //icon_texture is an SDL_Texture*

/// 3. Create a renderer
SDL_Renderer *sdl_renderer = SDL_CreateRenderer(/*...*/);
NapysRendererTTF *napys_renderer = NapysCreateRendererTTF(ctx, sdl_renderer);
```

After that, you can either create a command list and manually fill it, or use a built-in rich text parser to fill it for you instead.
Napys Parser uses a very simple syntax, where everything is considered as text, except for commands, surrounded by `{{` and `}}`. For example, to draw a title-sized text with green color you can do:

```c
NapysCommandList *cmd_list = NapysParseRichText("{{size:title}}{{color:green}}Hello World!", NULL);
```

After your command list is ready, you can execute it with any renderer:

```c
NapysExecuteCommandList(napys_renderer, cmd_list);
```

This will read and execute all commands in the list - at this point all context resources are resolved and internal textures or text objects are created, so this should be done only once unless you change the context or the command list.

Finally, to draw the rendered text, you can use:

```c
NapysRenderTTF(napys_renderer, 50, 50); ///draw at (50, 50) position
```

Don't forget to cleanup the resources when you are done:

```c
NapysDestroyCommandList(cmd_list);
NapysDestroyRendererTTF(napys_renderer);
NapysDestroyContext(ctx);
```

## Documentation

You can find the documentation in the header file [napys.h](include/napys.h).

## License

[MIT](LICENSE)
