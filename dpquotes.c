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
#include <wctype.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * dpquotes.c - Turn straight double quotes into directional quotes
 *
 * Project Gutenberg Distributed Proofreaders proofreads quotation marks
 * as straight quotes and then (optionally) they are converted into
 * directional quotation marks during post-processing.
 *
 * This filter converts straight quote marks into directional quote marks,
 * issuing a warning message (based on surrounding white space) if the
 * conversion seems incorrect. The warnings should be checked manually and
 * fixed up if necessary.
 *
 * Old books (common at Distributed Proofreaders) sometimes followed a
 * convention where there is an open quote at the beginning of every line
 * rather than just at the start of the quotation. We'll usually only need
 * to convert these to directional quotes if they are in a no-wrap (poetry)
 * paragraph. The -p option enables this old-style convention for quotation
 * marks.
 *
 * Quote conversion should usually be done after relocating footnotes, because
 * a footnote could appear in the middle of a block quotation.
 */

int main(int argc, char **argv)
{
static wchar_t buff[1024];
wchar_t *ptr;
int len;
int inside_quotes = 0;
int old_style = 0;
int c;

  setlocale(LC_ALL, getenv("LANG"));

  while ((c = getopt(argc, argv, "p")) > -1)
  {
    switch (c)
    {
      case 'p':
        old_style = 1;
        break;
    }
  }

  while (fgetws(buff, sizeof(buff), stdin) > 0)
  {
    len = wcslen(buff);

    /*
     * Strip <CR><LF> from the end of the line.
     * Note that the file may have DOS, not UNIX, <CR><LF> convention.
     */

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

    /* Strip trailing spaces */

    while ((len > 0) && (buff[len-1] == ' '))
    {
      buff[len-1] = '\0';
      len--;
    }

    if (len == 0)
      inside_quotes = 0;

    ptr = buff;

    while (*ptr)
    {
      if (*ptr == '"')
      {
        if (inside_quotes)
        {
          putwchar(0x201d);
          inside_quotes = 0;
          if ((ptr == buff) || (ptr[-1] == ' '))
          {
            fwprintf(stderr, L"Warning: smart quotes puts close quote after a space.\n");
            fwprintf(stderr, L"    %ls\n", buff);
          }
        }
        else
        {
          putwchar(0x201c);
          inside_quotes = 1;
          if ((ptr[1] == '\0') || (ptr[1] == ' '))
          {
            fwprintf(stderr, L"Warning: smart quotes puts open quote before a space.\n");
            fwprintf(stderr, L"    %ls\n", buff);
          }
        } 
      }
      else
      {
        putwchar(*ptr);
      }

      ptr++;
    }
    putwchar('\n');

    if (old_style)
      inside_quotes = 0;
  }
}
