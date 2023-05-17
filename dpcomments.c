/*-
 * Copyright (c) 2020 Michael Roe
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>

#define LINE_MAX 1024

#define TAG_OTHER 0
#define TAG_COMMENT 1

int depth = 0;

static int tag_stack[50];

void push_tag(int tag)
{
  if (depth < 50)
  {
    tag_stack[depth] = tag;
    depth++;
  }
}

int pop_tag()
{
  if (depth > 0)
  {
    depth--;
    return tag_stack[depth];
  }
  else
    return -1;
}

int main(argc, argv)
int argc;
char **argv;
{
wchar_t in_buff[LINE_MAX];
wchar_t out_buff[LINE_MAX];
int comment_mode = 0;
int comment_on_line = 0;
int len;
wchar_t *in_ptr;
wchar_t *out_ptr;
int tag;

  setlocale(LC_ALL, getenv("LANG"));

  while (fgetws(in_buff, LINE_MAX, stdin) > 0)
  {
    len = wcslen(in_buff);
    /* Strip <CR><LF> from the end of the line.
     * Note that the file may have DOS, not UNIX, <CR><LF> convention.
     */
    if (in_buff[len-1] == '\n')
    {
      in_buff[len-1] = '\0';
      len--;
    }
    if (in_buff[len-1] == '\r')
    {
      in_buff[len-1] = '\0';
      len--;
    }

    comment_on_line = comment_mode;
    in_ptr = in_buff;
    out_ptr = out_buff;

    while (*in_ptr)
    {
      if (wcsncmp(in_ptr, L"[**", 3) == 0)
      {
        comment_mode = 1;
        comment_on_line = 1;
        in_ptr += 3;
        push_tag(TAG_COMMENT);
      }
      else if (*in_ptr == L'[')
      {
        push_tag(TAG_OTHER);
        if (comment_mode == 0)
        {
          *out_ptr = *in_ptr;
          out_ptr++;
        }
        in_ptr++;
      }
      else if (*in_ptr == L']')
      {
        tag = pop_tag();
        in_ptr++;
        if (tag == TAG_COMMENT)
          comment_mode = 0;
        else if (comment_mode == 0)
        {
          *out_ptr = L']';
          out_ptr++;
        }
      }
      else if (comment_mode == 0)
      {
        *out_ptr = *in_ptr;
        out_ptr++;
        in_ptr++;
      }
      else
      {
        in_ptr++;
      }
    }
    *out_ptr = 0;
    len = wcslen(out_buff);
    while ((len > 0) && (out_buff[len-1] == L' '))
      len--;
    out_buff[len] = 0;
    if ((len > 0) || (comment_on_line == 0))
      wprintf(L"%S\n", out_buff);
  }
  return 0;
}
