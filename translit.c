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
 * translit.c
 *
 * Convert traliterated Greek back into the Greek alphabet
 */

/* TO DO: nch -> gamma chi */

#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>

#include "dptools.h"

static wchar_t greek_table[256];

#define GREEK_STATE_NULL 0
#define GREEK_STATE_LC_H 1
#define GREEK_STATE_UC_H 2
#define GREEK_STATE_LC_C 3
#define GREEK_STATE_UC_C 4
#define GREEK_STATE_LC_P 5
#define GREEK_STATE_UC_P 6
#define GREEK_STATE_LC_T 7
#define GREEK_STATE_UC_T 8
#define GREEK_STATE_LC_N 9
#define GREEK_STATE_UC_N 10
#define GREEK_STATE_LC_R 11
#define GREEK_STATE_UC_R 12
#define GREEK_STATE_LC_S 13
#define GREEK_STATE_LC_NC 14
#define GREEK_STATE_UC_NC 15

static int greek_state = GREEK_STATE_NULL;

void translit_init()
{
wchar_t buff[1024];
int len;
int count = 0;
int page = 1;
int newpage = 0;
int i;
int italic = 0;
wchar_t *start; 
wchar_t *end;
wchar_t *cp;

  /* Need to set the locale before can print wide characters to stdout */
/*  setlocale(LC_ALL, getenv("LANG")); */

  for (i=0;i<256;i++)
    greek_table[i] = (wchar_t) i;

  greek_table['h'] = '\'';
  greek_table['H'] = '\'';

  greek_table['a'] = 0x3b1;
  greek_table['b'] = 0x3b2;
  greek_table['g'] = 0x3b3;
  greek_table['d'] = 0x3b4;
  greek_table['e'] = 0x3b5;
  greek_table['z'] = 0x3b6;
  greek_table[0xea] = 0x3b7; 
  greek_table['i'] = 0x3b9;
  greek_table['k'] = 0x3ba;
  greek_table['l'] = 0x3bb;
  greek_table['m'] = 0x3bc;
  greek_table['n'] = 0x3bd;
  greek_table['x'] = 0x3be;
  greek_table['o'] = 0x3bf;
  greek_table['p'] = 0x3c0;
  greek_table['r'] = 0x3c1;
  greek_table['s'] = 0x3c2;
  greek_table['t'] = 0x3c4;
  greek_table['u'] = 0x3c5;
  greek_table['y'] = 0x3c5;
  greek_table['w'] = 0x3dd;
  greek_table[0xf4] = 0x3c9;

  greek_table['A'] = 0x391;
  greek_table['B'] = 0x392;
  greek_table['G'] = 0x393;
  greek_table['D'] = 0x394;
  greek_table['E'] = 0x395;
  greek_table['Z'] = 0x396;
  greek_table[0xca] = 0x397; 
  greek_table['I'] = 0x399;
  greek_table['K'] = 0x39a;
  greek_table['L'] = 0x39b;
  greek_table['M'] = 0x39c;
  greek_table['N'] = 0x39d;
  greek_table['X'] = 0x39e;
  greek_table['O'] = 0x39f;
  greek_table['P'] = 0x3a0;
  greek_table['R'] = 0x3a1;
  greek_table['S'] = 0x3a3;
  greek_table['T'] = 0x3a4;
  greek_table['U'] = 0x3a5;
  greek_table['Y'] = 0x3a5;
  greek_table['W'] = 0x3dc;
  greek_table[0xd4] = 0x3a9;

  greek_table['?'] = 0x37e;
  greek_table[';'] = 0x387;

  greek_table[0xcf] = 0x3aa; /* I with diaresis */
  greek_table[0xdc] = 0x3ab; /* U with diaresis */
  greek_table[0xef] = 0x3ca; /* i with diaresis */
  greek_table[0xfc] = 0x3cb; /* u with diaresus */
  
}

void write_greek_char(FILE *outfile, wchar_t c)
{
  switch (greek_state)
  {
    case GREEK_STATE_LC_N:
      if ((c == 'g') || (c == 'G')
         || (c == 'x') || (c == 'X')
         || (c == 'k') || (c == 'K'))
      {
        fputwc(0x3b3, outfile);
      }
      else if (c == 'c')
      {
        greek_state = GREEK_STATE_LC_NC;
        return;
      }
      else
      {
        fputwc(0x3bd, outfile);
      }
      greek_state = GREEK_STATE_NULL;
      break; 
    case GREEK_STATE_UC_N:
      if ((c == 'g') || (c == 'G')
         || (c == 'x') || (c == 'X')
         || (c == 'k') || (c == 'K'))
      {
        fputwc(0x393, outfile);
      }
      else if (c == 'C')
      {
        greek_state = GREEK_STATE_UC_NC;
        return;
      }
      else
      {
        fputwc(0x39d, outfile);
      }
      greek_state = GREEK_STATE_NULL;
      break; 
    case GREEK_STATE_LC_NC:
      if (c == 'h')
      {
        fputwc(0x3b3, outfile);
      }
      else
      {
        fputwc(0x3bd, outfile);
      }
      greek_state = GREEK_STATE_LC_C;
      break;
    case GREEK_STATE_UC_NC:
      if (c == 'H')
      {
        fputwc(0x393, outfile);
      }
      else
      {
        fputwc(0x39d, outfile);
      }
      greek_state = GREEK_STATE_UC_C; 
      break;
    case GREEK_STATE_LC_R:
      if ((c == 'h') || (c == 'H'))
      {
        fputwc(0x1fe5, outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else
      {
        fputwc(0x3c1, outfile);
        greek_state = GREEK_STATE_NULL;
      }
      break;
    case GREEK_STATE_UC_R:
      if ((c == 'h') || (c == 'H'))
      {
        fputwc(0x1fec,outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else
      {
        fputwc(0x3a1, outfile);
        greek_state = GREEK_STATE_NULL;
      }
      break;
    case GREEK_STATE_LC_P:
      if ((c == 'h') || (c == 'H'))
      {
        fputwc(0x03c6, outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else if ((c == 's') || (c == 'S'))
      {
        fputwc(0x03c8, outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else
      {
        fputwc(greek_table['p'], outfile);
        greek_state = GREEK_STATE_NULL;
      }
      break;
    case GREEK_STATE_UC_P:
      if ((c == 'h') || (c == 'H'))
      {
        fputwc(0x03a6, outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else if ((c == 's') || (c == 'S'))
      {
        fputwc(0x03a8, outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else
      {
        fputwc(greek_table['P'], outfile);
        greek_state = GREEK_STATE_NULL;
      }
      break;
    case GREEK_STATE_LC_S:
      if ((c == ' ') || (c == ',') || (c == '.') ||
          (c == '?') || (c == ';'))
        fputwc(0x3c2, outfile);
      else
        fputwc(0x3c3, outfile);
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_LC_T:
      if ((c == 'h') || (c == 'H'))
      {
        fputwc(0x3b8, outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else
        fputwc(greek_table['t'], outfile);
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_UC_T:
      if ((c == 'h') || (c == 'H'))
      {
        fputwc(0x398, outfile);
        greek_state = GREEK_STATE_NULL;
        return;
      }
      else
        fputwc(greek_table['T'], outfile);
      greek_state = GREEK_STATE_NULL;
      break;
    default:
      break;
  }

  switch (greek_state)
  {
    case GREEK_STATE_NULL:
      switch (c)
      {
        case 'c':
          greek_state = GREEK_STATE_LC_C;
          break;
        case 'C':
          greek_state = GREEK_STATE_UC_C;
          break;
        case 'h':
          greek_state = GREEK_STATE_LC_H;
          break;
        case 'H':
          greek_state = GREEK_STATE_UC_H;
          break;
        case 'n':
          greek_state = GREEK_STATE_LC_N;
          break;
        case 'N':
          greek_state = GREEK_STATE_UC_N;
          break;
        case 'p':
          greek_state = GREEK_STATE_LC_P;
          break;
        case 'P':
          greek_state = GREEK_STATE_UC_P;
          break;
        case 'r':
          greek_state = GREEK_STATE_LC_R;
          break;
        case 'R':
          greek_state = GREEK_STATE_UC_R;
          break;
        case 's':
          greek_state = GREEK_STATE_LC_S;
          break;
        case 't':
          greek_state = GREEK_STATE_LC_T;
          break;
        case 'T':
          greek_state = GREEK_STATE_UC_T;
          break;
        default:
          fwprintf(outfile, L"%lc", greek_table[c]);
          break;
      }
      break;
    case GREEK_STATE_LC_C:
      if ((c == 'h') || (c == 'H'))
        fwprintf(outfile, L"%lc", 0x3c7);
      else
        fwprintf(stderr, L"Unexpected character (%lc) after c in Greek transliteration.\n", c);
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_UC_C:
      if ((c == 'h') || (c == 'H'))
        fwprintf(outfile, L"%lc", 0x3a7);
      else
        fwprintf(stderr, L"Unexpected character (%lc) after c in Greek transliteration.\n", c);
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_LC_H:
      switch (c)
      {
        case 'a':
        case 'A':
          fwprintf(outfile, L"%lc", 0x1f01);
          break;
        case 'e':
        case 'E':
          fwprintf(outfile, L"%lc", 0x1f11);
          break;
        case 0xea:
        case 0xca:
          fwprintf(outfile, L"%lc", 0x1f21);
          break;
        case 'i':
        case 'I':
          fwprintf(outfile, L"%lc", 0x1f31);
          break;
        case 'o':
        case 'O':
          fwprintf(outfile, L"%lc", 0x1f41);
          break;
        case 'u':
        case 'U':
        case 'y':
        case 'Y':
          fwprintf(outfile, L"%lc", 0x1f51);
          break;
        case 0xf4:
        case 0xd4:
          fwprintf(outfile, L"%lc", 0x1f61);
          break;
        default:
          fwprintf(outfile, L"%lc", c);
          break;
      }
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_UC_H:
      switch (c)
      {
        case 'a':
        case 'A':
          fwprintf(outfile, L"%lc", 0x1f09);
          break;
        case 'e':
        case 'E':
          fwprintf(outfile, L"%lc", 0x1f19);
          break;
        case 0xea:
        case 0xca:
          fwprintf(outfile, L"%lc", 0x1f29);
          break;
        case 'i':
        case 'I':
          fwprintf(outfile, L"%lc", 0x1f39);
          break;
        case 'o':
        case 'O':
          fwprintf(outfile, L"%lc", 0x1f49);
          break;
        case 'u':
        case 'U':
        case 'y':
        case 'Y':
          fwprintf(outfile, L"%lc", 0x1f59);
          break;
        case 0xf4:
        case 0xd4:
          fwprintf(outfile, L"%lc", 0x1f69);
          break;
        default:
          fwprintf(outfile, L"%lc", c);
          break;
      }
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_LC_P:
      if ((c == 'h') || (c == 'H'))
        fwprintf(outfile, L"%lc", 0x03c6);
      else if ((c == 's') || (c == 'S'))
        fwprintf(outfile, L"%lc", 0x03c8);
      else
        fwprintf(outfile, L"%lc%lc", greek_table['p'], greek_table[c]);
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_UC_P:
      if ((c == 'h') || (c == 'H'))
        fwprintf(outfile, L"%lc", 0x03a6);
      else if ((c == 's') || (c == 'S'))
        fwprintf(outfile, L"%lc", 0x03a8);
      else
        fwprintf(outfile, L"%lc%lc", greek_table['P'], greek_table[c]);
      greek_state = GREEK_STATE_NULL;
      break;
    case GREEK_STATE_LC_T:
      if ((c == 'h') || (c == 'H'))
        fwprintf(outfile, L"%lc",  0x3b8);
      else
        fwprintf(outfile, L"%lc%lc", greek_table['t'], greek_table[c]);
      greek_state = GREEK_STATE_NULL;
      break; 
    case GREEK_STATE_UC_T:
      if ((c == 'h') || (c == 'H'))
        fwprintf(outfile, L"%lc",  0x398);
      else
        fwprintf(outfile, L"%lc%lc", greek_table['T'], greek_table[c]);
      greek_state = GREEK_STATE_NULL;
      break;

    default:
      fwprintf(stderr, L"Unexpected greek transliteration state.\n");
      break;
  }
}

void flush_greek(FILE *outfile)
{
  switch (greek_state)
  {
    case GREEK_STATE_LC_N:
      fputwc(greek_table['n'], outfile);
      break;
    case GREEK_STATE_UC_N:
      fputwc(greek_table['N'], outfile);
      break;
    case GREEK_STATE_LC_P:
      fputwc(greek_table['p'], outfile);
      break;
    case GREEK_STATE_UC_P:
      fputwc(greek_table['P'], outfile);
      break;
    case GREEK_STATE_LC_R:
      fputwc(greek_table['r'],outfile);
      break;
    case GREEK_STATE_UC_R:
      fputwc(greek_table['R'],outfile);
      break;
    case GREEK_STATE_LC_S:
      fputwc(0x3c2, outfile);
      break;
    case GREEK_STATE_LC_T:
      fputwc(greek_table['t'],outfile);
      break;
    case GREEK_STATE_UC_T:
      fputwc(greek_table['T'],outfile);
      break;
    case GREEK_STATE_NULL:
      break;
    default:
      fwprintf(stderr, L"Unexpected state at end of [Greek:] mode.\n");
      break;
  }
  greek_state = GREEK_STATE_NULL;
}

void write_greek(FILE *outfile, wchar_t *str)
{
   wchar_t *cp;

   cp = str;

   while (*cp != L'\0')
   {
     switch (*cp)
     {
       case 's': /* lower-case sigma at end of a word */
         if ((cp[1] == L' ') || (cp[1] == L'\0'))
           fwprintf(outfile, L"%lc", 0x03c2);
         else
           fwprintf(outfile, L"%lc", 0x03c3);
         cp++;
         break;
       case 'C': /* "CH" for chi */
         if ((cp[1] == L'h') || (cp[1]== L'H'))
         {
           fwprintf(outfile, L"%lc", 0x3a7);
           cp += 2;
         }
         else
         {
           fwprintf(outfile, L"%lc", greek_table[*cp]);
           cp++;
         }
         break;
       case 'c':
         if (cp[1] == L'h')
         {
           fwprintf(outfile, L"%lc", 0x3c7);
           cp += 2;
         }
         else
         {
           fwprintf(outfile, L"%lc", greek_table[*cp]);
           cp++;
         }
         break;
       case 'T': /* TH for theta */
         if ((cp[1] == L'h') || (cp[1] == L'H'))
         {
           fwprintf(outfile, L"%lc", 0x398);
           cp += 2;
         }
         else
         {
           fwprintf(outfile, L"%lc", greek_table[*cp]);
           cp++;
         }
         break;
       case 't':
         if (cp[1] == L'h')
         {
           fwprintf(outfile, L"%lc", 0x3b8);
           cp += 2;
         }
         else
         {
           fwprintf(outfile, L"%lc", greek_table[*cp]);
           cp++;
         }
         break;
       case 'P': /* PH for phi, PS for psi */
         if ((cp[1] == L'H') || (cp[1] == L'h'))
         {
           fwprintf(outfile, L"%lc", 0x03a6);
           cp += 2;
         }
         else if ((cp[1] == L'S') || (cp[1] == L's'))
         {
           fwprintf(outfile, L"%lc", 0x03a8);
           cp += 2;
         }
         else
         {
           fwprintf(outfile, L"%lc", greek_table[*cp]);
           cp++;
         }
         break;
       case 'p': 
         if (cp[1] == L'h')
         {
           fwprintf(outfile, L"%lc", 0x03c6);
           cp += 2;
         }
         else if ((cp[1] == L'S') || (cp[1] == L's'))
         {
           fwprintf(outfile, L"%lc", 0x03c8);
           cp += 2;
         }
         else
         {
           fwprintf(outfile, L"%lc", greek_table[*cp]);
           cp++;
         }
         break;
       default:
         fwprintf(outfile, L"%lc", greek_table[*cp]);
         cp++;
         break;
      }
   }
}
