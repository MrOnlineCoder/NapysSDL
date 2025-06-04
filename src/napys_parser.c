#include <napys.h>
#include "napys_internal.h"

static void NapysParseRichTextTag(const char *tag, NapysCommandList *cmd_list)
{
    const char *delimeter = SDL_strstr(tag, ":");

    const char *cmd_name = SDL_strndup(tag, delimeter - tag);
    const char *cmd_value = delimeter != NULL ? SDL_strndup(delimeter + 1, SDL_strlen(tag) - (delimeter - tag) - 1) : NULL;

    NapysCommand cmd = {0};
    cmd.type = NAPYS_COMMAND_TYPE_NONE;

    if (SDL_strncmp(cmd_name, "color", 5) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_SET_COLOR;
        cmd.data = SDL_strdup(cmd_value);
    }
    else if (SDL_strncmp(cmd_name, "font", 4) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_SET_FONT;
        cmd.data = SDL_strdup(cmd_value);
    }
    else if (SDL_strncmp(cmd_name, "size", 4) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_SET_SIZE;
        cmd.data = SDL_strdup(cmd_value);
    }
    else if (cmd_value && SDL_strlen(cmd_name) == 0 && SDL_strncmp(cmd_value, "newline", 7) == 0)
    {
        cmd.type = NAPYS_COMMAND_TYPE_NEWLINE;
    }
    else if (SDL_strncmp(cmd_name, "image", 5) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_DRAW_IMAGE;
        cmd.data = SDL_strdup(cmd_value);
    }
    else
    {
        cmd.type = NAPYS_COMMAND_TYPE_USE_STRING;
        cmd.data = SDL_strdup(cmd_name);
    }

    SDL_free((void *)cmd_name);
    SDL_free((void *)cmd_value);

    if (cmd.type != NAPYS_COMMAND_TYPE_NONE)
    {
        NapysAddCommand(cmd_list, cmd);
    }
}

NapysCommandList *NapysParseRichText(const char *text, const NapysRichTextOptions *options)
{
    NapysCommandList *cmd_list = NapysCreateCommandList();

    const char *left_tag = options && options->left_tag ? options->left_tag : "{{";
    const char *right_tag = options && options->right_tag ? options->right_tag : "}}";
    const bool treat_newline_chars_as_commands = options ? options->treat_newline_chars_as_commands : false;

    const int len = SDL_strlen(text);

    const int lt_len = SDL_strlen(left_tag);
    const int rt_len = SDL_strlen(right_tag);

    const char *cursor = text;

    const char *last_text_start = text;

    const char *buffer = SDL_calloc(1, len + 1);

    while (cursor < text + len)
    {
        const char cc = *cursor;
        const int index = cursor - text;

        if (cc == left_tag[0] && index + lt_len <= len)
        {
            if (SDL_strncmp(cursor, left_tag, lt_len) == 0)
            {
                // Skip the left tag
                cursor += SDL_strlen(left_tag);

                // Try to find the ending right tag
                const char *end_tag = SDL_strnstr(cursor, right_tag, len - index - lt_len);

                if (end_tag)
                {
                    // If tag found, add the last text segment before the tag
                    if (last_text_start < cursor - lt_len)
                    {
                        const char *text_content = SDL_strndup(last_text_start, cursor - lt_len - last_text_start);
                        NapysAddDrawTextCommand(cmd_list, text_content);
                        SDL_free((void *)text_content); // Add draw text command makes a copy of the string
                    }

                    // Add the tag command
                    const char *tag_content = SDL_strndup(cursor, end_tag - cursor);
                    NapysParseRichTextTag(tag_content, cmd_list);
                    SDL_free((void *)tag_content);

                    // Skip the right tag
                    cursor = end_tag + rt_len;
                    last_text_start = cursor;
                }
                else
                {
                    NapysSetError("Unmatched left tag in rich text");
                    NapysDestroyCommandList(cmd_list);
                    SDL_free((void *)buffer);
                    return NULL;
                }
            }
        }
        else if (treat_newline_chars_as_commands && cc == '\n')
        {
            // If newline character is treated as a command, add a newline command
            if (last_text_start < cursor)
            {
                const char *text_content = SDL_strndup(last_text_start, cursor - last_text_start);
                NapysAddDrawTextCommand(cmd_list, text_content);
                SDL_free((void *)text_content); // Add draw text command makes a copy of the string
            }

            NapysAddNewlineCommand(cmd_list);
            last_text_start = cursor + 1;
            cursor++;
        }
        else
        {
            cursor++;
        }
    }

    if (last_text_start < cursor && cursor < text + len)
    {
        // Add the last text segment after the last tag
        const char *text_content = SDL_strndup(last_text_start, cursor - last_text_start);
        NapysAddDrawTextCommand(cmd_list, text_content);
        SDL_free((void *)text_content); // Add draw text command makes a copy of the string
    }

    return cmd_list;
}
