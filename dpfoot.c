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
#include <getopt.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <stdlib.h>

#include "footnote.h"

#define MAX_BUFF 1024

struct footnote {
  struct footnote *next_footnote;
  wchar_t *line;
};

static struct footnote *notes = (struct footnote *) 0;
static struct footnote *last_footnote = (struct footnote *) 0;

static int renumber_numeric = 0;

void flush_footnotes()
{
struct footnote *ptr;

  ptr  = notes;
  while (ptr)
  {
    wprintf(L"%S\n", ptr->line);
    ptr = ptr->next_footnote;
  }
  /* TO DO: free() the memory */
  notes = (struct footnote *) 0;
  last_footnote = (struct footnote *) 0;
}

int add_footnote(buff)
wchar_t *buff;
{
struct footnote *new;

  new = (struct footnote *) malloc(sizeof(struct footnote));
  if (new == (struct footnote *) 0)
    return -1;
  new->line = (wchar_t *) malloc((wcslen(buff)+1)*sizeof(wchar_t));
  if (new->line == (wchar_t *) 0)
    return -1;
  wcscpy(new->line, buff);
  if (last_footnote)
    last_footnote->next_footnote = new;
  else
    notes = new;
  last_footnote = new;
}

int count_brackets(line)
wchar_t *line;
{
wchar_t *cp;
int depth;

  cp = line;
  depth = 0;
  while (*cp)
  {
    if (*cp == '[')
      depth++;
    else if (*cp == ']')
      depth--;
    cp++;
  }
  return depth;
}

void renumber(wchar_t *line, int *footmin, int *footmax)
{
#define MAX_DIGITS 10
wchar_t digits[MAX_DIGITS];
wchar_t tmp[MAX_BUFF];
wchar_t *cp1;
wchar_t *cp2;
wchar_t *cp3;
int footnum;
int touched = 0;
int num;
int len;

  /* NB: Potential buffer overflow if line too long */

  cp1 = line;
  cp2 = tmp;
  while (*cp1)
  {
    if (*cp1 != '[')
    {
      *cp2 = *cp1;
      cp1++;
      cp2++;
    }
    else if ((cp1[1] >= 'A') && (cp1[1] <= 'Z') && (cp1[2] == ']'))
    {
      touched = 1;

      *cp2 = *cp1; /* Copy the [ */
      cp1++;
      cp2++;
     
      footnum = *footmin + (*cp1 - 'A') + 1;
      if (footnum > *footmax)
        *footmax = footnum;

      swprintf(digits, MAX_DIGITS, L"%d", footnum);
      cp3 = digits;
      while (*cp3)
      {
        *cp2 = *cp3;
        cp2++;
        cp3++;
      }      
      cp1++;
      *cp2 = *cp1; /* Copy the ] */
      cp1++;
      cp2++;
    }
    else if (renumber_numeric && is_footnote(cp1, &num, &len))
    {
      touched = 1;

      *cp2 = *cp1; /* Copy the [ */
      cp1++;
      cp2++;
      footnum =  *footmin + num;
      if (footnum > *footmax)
        *footmax = footnum;
      swprintf(digits, sizeof(digits), L"%d", footnum);
      cp3 = digits;
      while (*cp3)
      {
        *cp2 = *cp3;
        cp2++;
        cp3++;
      }      
      cp1 += len-2;
      *cp2 = *cp1; /* Copy the ] */
      cp1++;
      cp2++;
    }
    else
    {
      *cp2 = *cp1;
      cp1++;
      cp2++;
    }
  }
  *cp2 = '\0';

  if (touched)
    wcscpy(line, tmp);
}

void renumber_footnote(wchar_t *line, int *footmin)
{
wchar_t val[10];
wchar_t *end_of_num;
int digits;
wchar_t tmp[MAX_BUFF];
wchar_t *cp1;
wchar_t *cp2;
int footnum;

  /* NB: Potential buffer overflow if line too long */

  if (wcsncmp(line, L"[Footnote:", 10) == 0)
  {
    /*An un-numbered footnote. No need to do anything. */
  }
  else if (wcsncmp(line, L"[Footnote ", 10) == 0)
  {
    cp1 = line + 10;
    while (*cp1 == ' ')
      cp1++;
    if ((*cp1 >= 'A') && (*cp1 <= 'Z') && (cp1[1] == ':'))
    {
      footnum = *footmin + (*cp1 - 'A') + 1;
      swprintf(tmp, MAX_BUFF, L"[Footnote %d:", footnum);
      cp2 = tmp + wcslen(tmp);
      cp1 += 2;
      while (*cp1)
      {
        *cp2 = *cp1;
        cp1++;
        cp2++;
      }
      *cp2 = '\0'; 
      wcscpy(line, tmp);
    }
    else if (renumber_numeric && iswdigit(*cp1))
    {
      digits = 0;
      while (iswdigit(*cp1) && (digits < 10))
      {
        val[digits] = *cp1;
        digits++;
        cp1++;
      }
      if (digits == 10)
      {
        fwprintf(stderr, L"Too many digits in footnote number.\n");
        digits = 9;
      }
      val[digits] = '\0';
      footnum = *footmin + wcstol(val, &end_of_num, 10);  
      swprintf(tmp, MAX_BUFF, L"[Footnote %d:", footnum);
      cp2 = tmp + wcslen(tmp);
      cp1++;
      while (*cp1)
      {
        *cp2 = *cp1;
        cp1++;
        cp2++;
      }
      *cp2 = '\0'; 
      wcscpy(line, tmp);
    }
  }
}

int main(argc, argv)
int argc;
char **argv;
{
int restart_section = 0;
int restart_chapter = 0;
int flush_section = 0;
int flush_chapter = 0;
int c;
wchar_t buff[MAX_BUFF];
int i;
int len;
int blank_lines = 0;
int footnote_mode = 0;
int bracket_depth = 0;
int footmax = 0; /* The highest footnote number that's been used so far */
int footmin = 0; /* The highest footnote number on pages before the current one*/
int number_pages = 0;
int newpage_pending = 0;
wchar_t page_name[MAX_BUFF];

  /* Need to set the locale before can print wide characters to stdout */
  setlocale(LC_ALL, getenv("LANG"));

  while ((c = getopt(argc, argv, "CSNcns")) > -1)
  {
    switch (c)
    {
      case 'S':
        restart_section = 1;
        break;
      case 'C':
        restart_chapter = 1;
        break;
      case 'N':
        renumber_numeric = 1;
        break;
      case 's':
        flush_section = 1;
        break;
      case 'c':
        flush_chapter = 1;
        break;
      case 'n':
        number_pages = 1;
        break;
    }
  } 

  if (flush_section)
    flush_chapter = 1; /* The end of a chapter is also the end of a section */

  if (restart_section)
    restart_chapter = 1;

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
      blank_lines++;
    else if (wcsncmp(buff, L"-----", 5) == 0) /* A page break */
    {
      if (number_pages)
      {
        wcscpy(page_name, buff);
        newpage_pending = 1;
      }

      blank_lines = 0;
      if (footnote_mode && (bracket_depth > 0))
      {
        fwprintf(stderr, L"Warning: footnote markup not closed by end of page\n");
        fwprintf(stderr, L"  %ls\n", buff);
      }
      footnote_mode = 0;
      bracket_depth = 0;
      footmin = footmax;
    }
    else
    {
       if (blank_lines > 0)
       {
         if (footnote_mode == 0)
         {
           if (blank_lines == 4) /* 4 blank lines -> chapter heading */
           {
             if (flush_chapter)
               flush_footnotes();
             if (restart_chapter)
             {
               footmin = 0;
               footmax = 0;
             }
           }
           else if (blank_lines == 2) /* 2 blank lines -> section heading or furher part of chapter heading */
           {
           /* Don't need to work out whether this is a section header or part
            * of a chapter header, because if it's part of a chapter header
            * the footnotes will have already been flushed, and flushing them
            * again is harmless.
            */
             if (flush_section)
               flush_footnotes();
             if (restart_section)
             {
               footmin = 0;
               footmax = 0;
             }
           }
         }
         else /* in footnote mode */
         {
           if (bracket_depth == 0)
             footnote_mode = 0; 
         }
         if (wcsncmp(buff, L"[Footnote", 9) == 0)
         {
           footnote_mode = 1;
         }
         else if (wcsncmp(buff, L"*[Footnote", 10) == 0)
         {
           footnote_mode = 1;
         }
         if (footnote_mode)
           add_footnote(L"");
         else
         {
           if (number_pages && newpage_pending)
           {
             wprintf(L"%ls\n", page_name);
             newpage_pending = 0;
           }
           for (i=0;i<blank_lines;i++)
             wprintf(L"\n");
         }
         blank_lines = 0;
       }
       if (footnote_mode)
       {
         renumber_footnote(buff, &footmin);
         add_footnote(buff);
         bracket_depth += count_brackets(buff);
       }
       else
       {
         if (number_pages && newpage_pending)
         {
           wprintf(L"%ls\n", page_name);
           newpage_pending = 0;
         }
         renumber(buff, &footmin, &footmax);
         wprintf(L"%S\n", buff);
       }
    }
  }
  flush_footnotes();
  return 0;
}

