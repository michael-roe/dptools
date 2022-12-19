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

#include <wchar.h>

#include "entity.h"

struct entity dp_entities[] = {
	L"[=a]", (wchar_t) 0x101, NULL, NULL,
        L"[=E]", (wchar_t) 0x112, NULL, NULL,
	L"[=e]", (wchar_t) 0x113, NULL, NULL,
	L"[e,]", (wchar_t) 0x119, NULL, NULL,
	L"[=i]", (wchar_t) 0x12B, NULL, NULL,
	L"[=o]", (wchar_t) 0x14d, NULL, NULL,
        L"[OE]", (wchar_t) 0x152, L"OE", L"&OElig;",
        L"[oe]", (wchar_t) 0x153, L"oe", L"&oelig;",
	L"[=u]", (wchar_t) 0x16b, NULL, NULL,
        L"[)U]", (wchar_t) 0x16c, NULL, NULL,
	L"[)u]", (wchar_t) 0x16d, NULL, NULL,
        L"[Gh]", (wchar_t) 0x21c, NULL, NULL,
        L"[gh]", (wchar_t) 0x21d, NULL, NULL,
        /* These are my additions, not DP standard */
        L"[osb]", (wchar_t) '[', L"[", L"[",
        L"[csb]", (wchar_t) ']', L"]", L"]",
        L"[ldquo]", (wchar_t) 0x201c, L"\"", L"&ldquo;",
        L"[rdquo]", (wchar_t) 0x201d, L"\"", L"&rdquo;",
        L"[lsquo]", (wchar_t) 0x2018, L"'", L"&lsquo;",
        L"[rsquo]", (wchar_t) 0x2019, L"'", L"&rsquo;",
        /* Greek iota subscripts */
        L"[a_i]", (wchar_t) 0x1fb3, L"\u00e2i", NULL,
	L"[\xea_i]", (wchar_t) 0x1fc3, L"\u00eai", NULL,
	L"[\xf4_i]", (wchar_t) 0x1ff3, L"\u00f4i", NULL,
        /* Other Greek characters */
        L"[']", (wchar_t) 0x374, L"'", NULL,
        L"[st]", (wchar_t) 0x3db, NULL, NULL,
        L"[ST]", (wchar_t) 0x3da, NULL, NULL,
	L"", (wchar_t) 0, NULL, NULL
};

struct entity *find_entity(str, lptr)
wchar_t *str;
int *lptr;
{
struct entity *ptr;
int len;
wchar_t *cp;

  cp = str;
  len = 1;

  while ((*cp != L']') && (*cp != 0))
  {
    cp++;
    len++;
  }

  if (*cp == 0)
    return (struct entity *) 0;

  ptr = dp_entities;
  while (ptr->unicode)
  {
    if (wcsncmp(ptr->name, str, len) == 0)
    {
      *lptr = len;
      return ptr;
    }
    ptr++;
  }
  return (struct entity *) 0;
}
