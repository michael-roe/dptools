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

/*
 * dptxt.c - format Distributed Proofreaders markup as a plain text file
 */

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <stdlib.h>
#include <getopt.h>

#include "entity.h"

/*
 * TO DO:
 *
 * Skip comments [** ]
 */

static wchar_t rbuff[32768];
static wchar_t *rptr = rbuff;
static int inbuff = 0;
static int last_indent = 0;

void rewrap(indent, line)
int indent;
wchar_t *line;
{
wchar_t *cp;
int len;
int i;
int todo;

  last_indent = indent;

  len = wcslen(line);
/*  wprintf(L"[%ls]\n", line); */
  if (rptr != rbuff)
  {
    *rptr = L' ';
    rptr++;
    inbuff++;
  }
  wcscpy(rptr, line);
  rptr += len;
  inbuff += len;
  while (inbuff >= 70-indent)
  {
    todo = 70-indent;
    while ((todo > 0) && (rbuff[todo-1] != ' '))
      todo--;
    if (todo == 0)
      todo = 70-indent;
    /* wprintf(L"Printing line\n"); */
    for (i=0;i<indent;i++)
      wprintf(L" ");
    for (i=0;i<todo-1;i++)
      wprintf(L"%lc", rbuff[i]);
    if (rbuff[todo-1] != ' ')
      wprintf(L"%lc", rbuff[todo-1]);
    wprintf(L"\n");
    inbuff -= todo;
    cp = rbuff + todo;
    while (*cp)
    {
      cp[-todo] = *cp;
      cp++;
    }
    cp[-todo] = 0;
    rptr -= todo;
  }
} 

void rflush()
{
int i;

  if (inbuff > 70-last_indent)
    fwprintf(stderr, L"More than 70-indent characters in buffer!\n");

  if (inbuff != 0)
  {
    for (i=0;i<last_indent;i++)
      wprintf(L" ");
    wprintf(L"%ls\n", rbuff);
  }

  inbuff = 0;
  rptr = rbuff;
  rbuff[0] = 0;
  wprintf(L"\n");
}

static int poetry_indent2 = 6;
static int poetry_limit = 70;

void rewrap_poem(indent, line)
int indent;
wchar_t *line;
{
int len;
int i;
int todo;
wchar_t *ptr;

  len = wcslen(line);
  if (len+indent <= poetry_limit)
  {
    for (i=0;i<indent;i++)
      wprintf(L" ");
    wprintf(L"%ls\n", line);
  }
  else
  {
    todo = 70-indent;
    while ((todo > 0) && (line[todo-1] != L' '))
      todo--;
    if (todo == 0)
      todo = 70-indent;

    for (i=0;i<indent;i++)
      wprintf(L" ");
    for (i=0;i<todo;i++)
      wprintf(L"%lc", line[i]);
    wprintf(L"\n");

    ptr = line+todo;
    len -= todo;

    while (len)
    {
      if (len <= 70-poetry_indent2)
      {
        todo = len;
      }
      else
      {
        todo = 70-poetry_indent2;
        while ((todo > 0) && (ptr[todo-1] != L' '))
          todo--;
        if (todo == 0)
          todo = 70-poetry_indent2;
      } 
  
      for (i=0;i<poetry_indent2;i++)
        wprintf(L" ");
      for (i=0;i<todo;i++)
        wprintf(L"%lc", ptr[i]);
      wprintf(L"\n");
      ptr += todo;
      len -= todo;
    }
  }
}

void format_command(cpp)
wchar_t **cpp;
{
wchar_t *ptr;

  ptr = *cpp;
  while ((*ptr != 0) && (*ptr != ']'))
  {
    ptr++;
  }
  if (*ptr == 0)
  {
    fwprintf(stderr, L"Malformed [Format: command.\n");
    *cpp = ptr;
  }
  else
  {
    poetry_indent2 = 20; /* HACK */
    *cpp = ptr+1;
  }
}

int main(argc, argv)
int argc;
char **argv;
{
static wchar_t *hrule = L"     *     *     *     *     *";
int poetry_mode = 0;
int quote_mode = 0;
int caps_mode = 0;
wchar_t buff[1024];
wchar_t line[1024];
wchar_t *cp1;
wchar_t *cp2;
int len;
int quote_indent = 4;
int poetry_indent = 2;
int i;
int l;
struct entity *e;
int c;
int expand_entities = 0;

  setlocale(LC_ALL, getenv("LANG"));

  while ((c = getopt(argc, argv, "ep:q:r:l:")) > -1)
  {
    switch (c)
    {
      case 'e':
        expand_entities = 1;
        break;
      case 'l':
        poetry_limit = atoi(optarg);
        break;
      case 'p':
        poetry_indent = atoi(optarg);
        break;
      case 'q':
        quote_indent = atoi(optarg);
        break;
      case 'r':
        poetry_indent2 = atoi(optarg);
        break;
    }
  }

  rptr = rbuff;

  while (fgetws(buff, sizeof(buff), stdin) > 0)
  {
    len = wcslen(buff);

    if (buff[len-1] == '\n')
    {
      buff[len-1] = '\0';
      len--;
    }
    if (buff[len-1] == '\r')
    {
      buff[len-1] = '\0';
      len--;
    }

    if (wcscmp(buff, L"/*") == 0) 
    {
      poetry_mode = 1;
    }
    else if (wcscmp(buff, L"*/") == 0)
    {
      poetry_mode = 0;
    }
    else if (wcscmp(buff, L"/#") == 0)
    {
      quote_mode = 1;
    }
    else if (wcscmp(buff, L"#/") == 0)
    {
      quote_mode = 0;
    }
    else if (wcsncmp(buff, L"-----File", 9) == 0)
    {
    }
    else if (wcscmp(buff, L"[Blank Page]") == 0)
    {
    }
    else if (buff[0] == 0)
    {
      if (poetry_mode == 0)
      {
        if (quote_mode)
          rflush(quote_indent);
        else
          rflush(0);
      }
      else
        wprintf(L"\n");
    }
    else
    {
      cp1 = buff;
      cp2 = line;
      while (*cp1)
      {
        switch (*cp1)
        {
          case '\r':
          case '\n':
            cp1++;
            break;
          case '<':
            if (wcsncmp(cp1, L"<i>", 3) == 0)
            {
              *cp2 = '_';
              cp1 += 3;
              cp2++;
            }
            else if (wcsncmp(cp1, L"</i>", 4) == 0)
            {
              *cp2 = '_';
              cp1 += 4;
              cp2++;
            }
            else if (wcsncmp(cp1, L"<b>", 3) == 0)
            {
              *cp2 = '*';
              cp1 += 3;
              cp2++;
            }
            else if (wcsncmp(cp1, L"</b>", 4) == 0)
            {
              *cp2 = '*';
              cp1 += 4;
              cp2++;
            }
            else if (wcsncmp(cp1, L"<sc>", 4) == 0)
            {
              cp1 += 4;
              caps_mode = 1;
            }
            else if (wcsncmp(cp1, L"</sc>", 5) == 0)
            {
              cp1 += 5;
              caps_mode = 0;
            }
            else if (wcsncmp(cp1, L"<f>", 3) == 0)
            {
              *cp2 = '*';
              cp1 += 3;
              cp2++;
            }
            else if (wcsncmp(cp1, L"</f>", 4) == 0)
            {
              *cp2 = '*';
              cp1 += 4;
              cp2++;
            }
            else if (wcsncmp(cp1, L"<tb>", 4) == 0)
            {
              wcscpy(cp2, hrule);
              cp2 += wcslen(hrule);
              cp1 += 4;
            }
            else if (wcsncmp(cp1, L"<u>", 3) == 0)
            {
              *cp2 = '_';
              cp2++;
              cp1 += 3;
            }
            else if (wcsncmp(cp1, L"</u>", 4) == 0)
            {
              *cp2 = '_';
              cp2++;
              cp1 += 4;
            }
            else
            {
              fwprintf(stderr, L"Unexpected markup!\n");
              fwprintf(stderr, L"%ls\n", cp1);
              *cp2 = *cp1;
              cp1++;
              cp2++;
            }
            break;
          case '[':
            if (wcsncmp(L"[Format:", cp1, 8) == 0)
            {
              format_command(&cp1);
            }
            else if (e = find_entity(cp1, &l))
            {
              cp1 += l;
              if (expand_entities)
              {
                *cp2 = e->unicode;
                cp2++;
              }
              else if (e->latin1)
              {
                wcscpy(cp2, e->latin1);
                cp2 += wcslen(e->latin1);
              }
              else
              {
                wcscpy(cp2, e->name);
                cp2 += wcslen(e->name);
              }
            }
            else
            {
              *cp2 = *cp1;
              cp1++;
              cp2++; 
            }
            break; 
          default:
            if (caps_mode)
              *cp2 = towupper(*cp1);
            else
              *cp2 = *cp1;
            cp1++;
            cp2++;
            break; 
        }
      }
      *cp2 = 0;
      if (poetry_mode)
      {
        if (quote_mode)
          rewrap_poem(poetry_indent+quote_indent, line);
        else
          rewrap_poem(poetry_indent, line);
      }
      else if (quote_mode)
      {
        rewrap(quote_indent, line);
      }
      else
      {
        rewrap(0, line);
      }
    }
  }
  rflush(0);
  return 0;
}
