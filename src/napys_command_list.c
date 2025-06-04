#include <napys.h>
#include "napys_internal.h"

NapysCommandList *NapysCreateCommandList()
{
    NapysCommandList *list = SDL_malloc(sizeof(NapysCommandList));

    if (!list)
    {
        NapysSetError("Failed to allocate memory for command list");
        return NULL;
    }

    list->cmds = NULL;
    list->cmd_count = 0;
    list->cmd_capacity = 0;

    return list;
}

void NapysClearCommandList(NapysCommandList *list)
{
    if (list)
    {
        for (int i = 0; i < list->cmd_count; i++)
        {
            if (list->cmds[i].data)
            {
                SDL_free(list->cmds[i].data);
            }
        }
        list->cmd_count = 0;
    }
}

void NapysDestroyCommandList(NapysCommandList *list)
{
    if (list != NULL)
    {
        if (list->cmds != NULL)
        {
            NapysClearCommandList(list);
            SDL_free(list->cmds);
        }
        SDL_free(list);
    }
}

bool NapysAddCommand(NapysCommandList *list, NapysCommand cmd)
{
    if (!list)
    {
        return NapysSetError("Invalid command list");
    }

    if (list->cmd_count >= list->cmd_capacity)
    {
        int new_capacity = list->cmd_capacity == 0 ? 4 : list->cmd_capacity * 2;
        NapysCommand *new_cmds = SDL_realloc(list->cmds, new_capacity * sizeof(NapysCommand));
        if (!new_cmds)
        {
            return NapysSetError("Failed to allocate memory for commands");
        }
        list->cmds = new_cmds;
        list->cmd_capacity = new_capacity;
    }

    list->cmds[list->cmd_count] = cmd;
    list->cmd_count++;

    return true;
}

bool NapysAddDrawTextCommand(NapysCommandList *list, const char *text)
{
    if (!list || !text)
    {
        return NapysSetError("Invalid command list or text");
    }

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_DRAW_TEXT;
    cmd.data = SDL_strdup(text);

    return NapysAddCommand(list, cmd);
}

bool NapysAddSetColorCommand(NapysCommandList *list, const char *color_name)
{
    if (!list || !color_name)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_SET_COLOR;
    cmd.data = SDL_strdup(color_name);

    return NapysAddCommand(list, cmd);
}

bool NapysAddSetFontCommand(NapysCommandList *list, const char *color_name)
{
    if (!list || !color_name)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_SET_FONT;
    cmd.data = SDL_strdup(color_name);

    return NapysAddCommand(list, cmd);
}

bool NapysAddSetSizeCommand(NapysCommandList *list, const char *size_name)
{
    if (!list || !size_name)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_SET_SIZE;
    cmd.data = SDL_strdup(size_name);

    return NapysAddCommand(list, cmd);
}

bool NapysAddNewlineCommand(NapysCommandList *list)
{
    if (!list)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_NEWLINE;
    cmd.data = NULL;

    return NapysAddCommand(list, cmd);
}

bool NapysAddDrawImageCommand(NapysCommandList *list, const char *image_name)
{
    if (!list || !image_name)
        return NapysSetError("Invalid command list or image name");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_DRAW_IMAGE;
    cmd.data = SDL_strdup(image_name);

    return NapysAddCommand(list, cmd);
}

bool NapysAddUseStringCommand(NapysCommandList *list, const char *key)
{
    if (!list || !key)
        return NapysSetError("Invalid command list or key");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_USE_STRING;
    cmd.data = SDL_strdup(key);

    return NapysAddCommand(list, cmd);
}