#ifndef NAPYS_H
#define NAPYS_H

#include <SDL3/SDL.h>

#include <SDL3_ttf/SDL_ttf.h>

/**
 * Maximum supported font size.
 * This is used to limit the number of font sizes cached, as they are stored in a plain array.
 */

#define NAPYS_MAX_FONT_SIZE 256

/**
 * Maximum number of text fragments that can be rendered at once.
 */
#define NAPYS_TTF_RENDERER_MAX_TEXTS 128

/**
 * Opaque handle for hashmap implementation.
 */
typedef struct NapysHashmap NapysHashmap;

/**
 * Font cache, storing all available sizes for a given TTF font.
 */
typedef struct
{
    TTF_Font *sizes[NAPYS_MAX_FONT_SIZE];
    TTF_Font *base;
} NapysFontCache;

/**
 * Internal type for registry entry type.
 */
typedef enum
{
    NAPYS_REGISTRY_ENTRY_NONE,
    NAPYS_REGISTRY_ENTRY_STRING,
    NAPYS_REGISTRY_ENTRY_IMAGE,
    NAPYS_REGISTRY_ENTRY_COLOR,
    NAPYS_REGISTRY_ENTRY_SIZE
} NapysRegistryEntryType;

/**
 * Context registry entry
 */
typedef struct
{
    NapysRegistryEntryType type;

    char *str;
    SDL_Color color;
    int ptsize;
    void *img;
} NapysRegistryEntry;

/**
 * Napys context, containing the registry and font caches.
 */
typedef struct
{
    NapysHashmap *registry;
    NapysHashmap *fonts;

    NapysFontCache *default_font_cache;
} NapysContext;

/**
 * Get the last error message from Napys.
 *
 * Note, this works as SDL_GetError() - the function may return a message even last operation was successful.
 * Always use the return value of the called function for determining success or failure first.
 *
 * @return A pointer to the last error message, or NULL if no error occurred.
 */
const char *NapysGetError();

/**
 * Create a new Napys context.asm
 *
 * Each context contains an independent registry of colors, sizes, fonts, images and other resources
 * that can be used in Napys commands during rendering.
 *
 * At least one context is required to use Napys.
 *
 * @return A pointer to the newly created Napys context, or NULL if an error occurred.
 */
NapysContext *NapysCreateContext();

/**
 * Register a TTF font in the Napys context.
 *
 * The font will be cached and can be used in Napys commands using the assigned name.
 * The size of provided TTF_Font does not matter, as Napys will create copies of the font when a different size is requested.
 * The supplied TTF_Font must be a valid font until the context is destroyed.
 * Registering a font under the same name will trigger an error.
 * The first font registered will be used as the default font for the context.
 *
 * You can provide NULL as the name to try detect the font family name automatically, however it may not be always successful.
 *
 * @param ctx The Napys context to register the font in.
 * @param font The TTF_Font to register.
 * @param name Optional name for the font, if NULL, the font family name will be used when possible.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysRegisterFont(NapysContext *ctx, TTF_Font *font, const char *name);

bool NapysRegisterString(NapysContext *ctx, const char *key, const char *value);

/**
 * Register a color in the Napys context.
 * The color can be used in Napys commands using the assigned key.
 * The color is stored as an SDL_Color structure.
 *
 * @param ctx The Napys context to register the color in.
 * @param key The key to register the color under.
 * @param color The SDL_Color to register.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysRegisterColor(NapysContext *ctx, const char *key, SDL_Color color);

/**
 * Register common CSS colors in the Napys context.
 *
 * This function registers a set of common CSS colors:
 * black, white, red, green, blue, yellow, cyan, magenta, gray, darkred, darkgreen, darkblue, darkgray,
 * lightgray, orange, purple, pink, brown, gold, silver, lightblue, lightgreen
 *
 * @param ctx The Napys context to register the colors in.
 */
void NapysRegisterCSSColors(NapysContext *ctx);

/**
 * Register a font size in the Napys context.
 *
 * The size is registered under the specified key and can be used in Napys commands.
 *
 * @param ctx The Napys context to register the size in.
 * @param key The key to register the size under.
 * @param pt The point size to register.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysRegisterSize(NapysContext *ctx, const char *key, int pt);

/**
 * Register an image in the Napys context.
 *
 * The image can be used in Napys commands using the assigned key.
 * For TTF/TextEngine renderer, the image must be a valid SDL_Texture pointer.
 *
 * The image is not copied and must remain valid until the context is destroyed.
 *
 * @param ctx The Napys context to register the image in.
 * @param key The key to register the image under.
 * @param img The SDL_Texture to register.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysRegisterImage(NapysContext *ctx, const char *key, void *img);

/**
 * Destroy a Napys context.
 *
 * This will free all resources associated with the context, including the registry and font caches.
 * After this call, the context pointer will be invalid.
 * None of the registered resources are freed, including the base handles for fonts - you must free them manually if needed.
 *
 * @param ctx The Napys context to destroy.
 */
void NapysDestroyContext(NapysContext *ctx);

/**
 * Command types for Napys.
 * These types are used to identify the type of command in a command list.
 */
typedef enum
{
    NAPYS_COMMAND_TYPE_NONE,
    NAPYS_COMMAND_TYPE_DRAW_TEXT,
    NAPYS_COMMAND_TYPE_SET_COLOR,
    NAPYS_COMMAND_TYPE_DRAW_IMAGE,
    NAPYS_COMMAND_TYPE_SET_FONT,
    NAPYS_COMMAND_TYPE_SET_SIZE,
    NAPYS_COMMAND_TYPE_NEWLINE,
} NapysCommandType;

/**
 * Command structure for Napys.
 */
typedef struct
{
    NapysCommandType type;
    char *data;
} NapysCommand;

/**
 * Napys command list
 *
 * Command lists contains sequences of instructions to perform during rendering, either resolving to
 * drawing text/image or changing on of the styling parameters.
 */
typedef struct
{
    NapysCommand *cmds;
    int cmd_count;
    int cmd_capacity;
} NapysCommandList;

/**
 * Create a new Napys command list.
 *
 * Command lists are used to store sequences of commands that can be executed by the renderer.
 * The command list is initially empty and can be filled with commands using the provided functions.
 *
 * @return A pointer to the newly created command list, or NULL if an error occurred.
 */
NapysCommandList *NapysCreateCommandList();

/**
 * Clear the command list, removing all commands.
 *
 * This function will free the memory used by the commands in the list, but not the list itself.
 * After this call, the command list will be empty.
 *
 * @param list The command list to clear.
 */
void NapysClearCommandList(NapysCommandList *list);

/**
 * Destroy a Napys command list.
 *
 * This will free all resources associated with the command list, including the commands themselves.
 * After this call, the command list pointer will be invalid.
 *
 * @param list The command list to destroy.
 */
void NapysDestroyCommandList(NapysCommandList *list);

/**
 * Add a command to the Napys command list.
 *
 * This function will add the specified command to the end of the command list.
 * If the command list is full, it will be resized to accommodate the new command.
 *
 * @param list The command list to add the command to.
 * @param cmd The command to add (copied on addition).
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysAddCommand(NapysCommandList *list, NapysCommand cmd);

/**
 * Shortcut function to add a draw text command to the command list.
 *
 * @param list The command list to add the command to.
 * @param text The text to draw. The string is copied, so it can be freed after this call.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysAddDrawTextCommand(NapysCommandList *list, const char *text);

/**
 * Add a set color command to the command list.
 *
 * This function will add a command to set the current drawing color to the specified color name.
 * All subsequent drawing commands will use this color until a new color is set.
 * The color must be registered in the context registry before command list is executed.
 * Executing a command with an unregistered color will have no effect.
 *
 * @param list The command list to add the command to.
 * @param color_name The name of the color to set. Must be registered in the context at the time of execution.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysAddSetColorCommand(NapysCommandList *list, const char *color_name);

/**
 * Add a set font command to the command list.
 *
 * This function will add a command to set the current font to the specified font name.
 * All subsequent text drawing commands will use this font until another set font command is issued.
 * The font must be registered in the context registry before command list is executed.
 * Executing a command with an unregistered font will have no effect.
 *
 * @param list The command list to add the command to.
 * @param color_name The name of the font to set. Must be registered in the context at the time of execution.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysAddSetFontCommand(NapysCommandList *list, const char *color_name);

/**
 * Add a set size command to the command list.
 *
 * This function will add a command to set the current font size to the specified size name.
 * All subsequent text drawing commands will use this size until another set size command is issued.
 * The size must be registered in the context registry before command list is executed.
 * Executing a command with an unregistered size will have no effect.
 *
 * @param list The command list to add the command to.
 * @param size_name The name of the size to set. Must be registered in the context at the time of execution.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysAddSetSizeCommand(NapysCommandList *list, const char *size_name);

/**
 * Add a newline command to the command list.
 *
 * This function will add a command to move the drawing position to the next line.
 * The next text drawing command will start at the beginning of the new line.
 *
 * @param list The command list to add the command to.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysAddNewlineCommand(NapysCommandList *list);

/**
 * Add a draw image command to the command list.
 *
 * This function will add a command to draw an image at the current drawing position.
 * The image must be registered in the context registry before command list is executed.
 * The image will be drawn inline, centred vertically with the current font size.
 * Executing a command with an unregistered image will have no effect.
 *
 * @param list The command list to add the command to.
 * @param image_name The name of the image to draw. Must be registered in the context at the time of execution.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysAddDrawImageCommand(NapysCommandList *list, const char *image_name);

typedef struct
{
    TTF_Text *text;
    SDL_Texture *img;
    int x;
    int y;
} NapysFragmentTTF;

/**
 * Napys SDL TTF renderer.
 *
 * Please do not use this structure directly, use the provided functions to create and manage the renderer.
 */
typedef struct
{
    NapysContext *ctx;
    TTF_TextEngine *engine;
    SDL_Renderer *sdl_renderer;

    NapysFragmentTTF fragments[NAPYS_TTF_RENDERER_MAX_TEXTS];
    int fragments_count;
    int fragment_pointer;

    SDL_Color current_color;
    TTF_Font *current_font;
    NapysFontCache *current_font_cache;
    int current_font_size;

    int draw_x;
    int draw_y;

    SDL_Rect bounds;
} NapysRendererTTF;

/**
 * Create a new Napys TTF renderer.
 *
 * This function creates a new renderer that can be used to render TTF text using the Napys context.
 * The renderer is initialized with the default font cache and the provided SDL_Renderer.
 * NapysRendererTTF works by creating multiple TTF_Text objects, arranging them in one big text.
 * This will also create a TTF_TextEngine for rendering the text under the hood.
 *
 * @param ctx The Napys context to use for rendering.
 * @param renderer The SDL_Renderer to use for rendering.
 * @return A pointer to the newly created NapysRendererTTF, or NULL if an error occurred.
 */
NapysRendererTTF *NapysCreateRendererTTF(NapysContext *ctx, SDL_Renderer *renderer);

/**
 * Destroy a Napys TTF renderer.
 *
 * This will free all resources associated with the renderer, including the TTF_TextEngine and all text fragments.
 * After this call, the renderer pointer will be invalid.
 * The TTF_TextEngine will be destroyed, but none of the registered resources in the context will be freed.
 *
 * @param renderer The NapysRendererTTF to destroy.
 */
void NapysDestroyRendererTTF(NapysRendererTTF *renderer);

/**
 * Execute a command list with the Napys TTF renderer.
 *
 * This function will process the commands in the command list and "render" the text and images
 * according to the commands internally. It will also update the current drawing position and bounds.
 * Executing a command list will reset the renderer state, so all your commands should be in the same list,
 * if you want to render multiple texts or images in one go.
 * After command list is executed, the result can be rendered every frame using NapysRenderTTF().
 *
 * @param renderer The NapysRendererTTF to use for rendering.
 * @param list The command list to execute. The commands in the list will be processed and rendered.
 */
void NapysExecuteCommandList(NapysRendererTTF *renderer, NapysCommandList *list);

/**
 * Render the command list execution result.
 *
 * This function will render the text and images that were processed during command list execution.
 * It will draw the text fragments and images at the specified position (x, y).
 * The text fragments will be drawn in the order they were added to the command list.
 * This function can be called every frame to render the text and images.
 *
 * @param renderer The NapysRendererTTF to use for rendering.
 *
 * @param x The x position to render the text at.
 * @param y The y position to render the text at.
 */
void NapysRenderTTF(NapysRendererTTF *renderer, float x, float y);

/**
 * Get the bounds of the rendered text.
 *
 * This function will return the bounds of the rendered text, which is the rectangle that contains all the text fragments and inline images.
 * The bounds are updated during command list execution, so you should call this function after executing the command list.
 * The bounds are returned in the output parameter.
 * @param renderer The NapysRendererTTF to get the bounds from.
 * @param output The SDL_Rect to store the bounds in. If NULL, the bounds will not be returned.
 * @return true on success, false on failure (use NapysGetError() to get the error message).
 */
bool NapysGetRenderedTextBounds(NapysRendererTTF *renderer, SDL_Rect *output);

/**
 * Options for parsing rich text.
 */
typedef struct
{
    const char *left_tag;  ///< The left tag to use for rich text parsing, default is "{{"
    const char *right_tag; ///< The right tag to use for rich text parsing, default is "}}"
} NapysRichTextOptions;

/**
 * Parse rich text and create a command list.
 *
 * This function will parse the provided rich text string and create a command list
 * that can be executed by the Napys TTF renderer.
 *
 * The syntax for rich text is:
 * 1. Segments surrounded by left and right tags (default "{{" and "}}") are treated as commands.
 * 2. Everything else is treated as plain text and will be drawn as is.
 *
 * Supported commands are:
 * - {{font:<font_name>}} - Set the font to the specified font name.
 * - {{color:<color_name>}} - Set the drawing color to the specified color name.
 * - {{size:<size_name>}} - Set the font size to the specified size name.
 * - {{image:<image_name>}} - Draw an image at the current position, the image must be registered in the context.
 * - {{newline}} - Move the drawing position to the next line.
 *
 * The requested resources shall be registered in the Napys context before executing the command list.
 *
 * @param text The rich text string to parse.
 * @param options Optional options for parsing rich text, can be NULL to use defaults.
 *
 * @return A pointer to the created command list, or NULL if an error occurred (use NapysGetError() to get the error message).
 */
NapysCommandList *NapysParseRichText(const char *text, const NapysRichTextOptions *options);

#endif