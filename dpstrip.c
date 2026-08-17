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
 * dpstrip.c - Pre-process files for Distributed Proofreaders
 *
 * Reduce multiple spaces to a single space
 * Remove spaces before stops
 * Make sure there is a space after a stop
 *
 * TO DO: no space before a dash
 */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

int main(int argc, char **argv)
{
wchar_t c;
int spaces = 0;
int stops = 0;
int dos_mode = 1;

  setlocale(LC_ALL, getenv("LANG"));

  while ((c = getwchar())>=0)
  {
    switch (c)
    {
      case ' ':
        spaces++;
        break;
      case '\r':
        break;
      case '\n':
        spaces=0;
        stops=0;
        if (dos_mode)
           putwchar('\r');
        putwchar('\n');
        break;
      case '.':
      case ',':
      case '!':
      case '?':
      case ':':
      case ';':
      case ')':
        stops = 1;
        spaces = 0;
        putwchar(c);
        break;
      case '"':
        if (spaces)
          putwchar(' ');
        putwchar(c);
        spaces = 0;
        stops = 0;
        break;
      default:
        if (spaces || stops)
          putwchar(' ');
        putwchar(c);
        spaces = 0;
        stops = 0;
        break;
    }
  }
}
