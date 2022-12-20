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
 * dphtml.c
 */

#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdlib.h>
#include <getopt.h>

#include "dptools.h"

/*
 * To Do:
 *
 * Commment syntax: [** ]
 * Greek transliteration: [Greek: ]
 * Footnotes: [Footnote 1: ]
 * Footnote references: [*], [A]
 * [Blank Page]
 * Rejoining words split over a page break
 * Many character entities
 * Line numbers in the margins of poems
 * Automatically put in anchors for page numbers
 */

#define PAR_TYPE_NONE 0
#define PAR_TYPE_NORMAL 1
#define PAR_TYPE_CHAPTER 2
#define PAR_TYPE_CHAPTER_A 3
#define PAR_TYPE_SECTION 4
#define PAR_TYPE_RULE 5

static FILE *outfile;

static int poetry_mode = 0;
static int quote_mode = 0;
static int footnote_mode = 0; /* This is not the same as footnote_mode in output.c */
static int sidenote_mode = 0;
static int blank_lines = 0;
static int par_type = 0;
static wchar_t buff[1024];
int chapter = 0;
int chapter_offset = 0;
int section = 0;
int number_sections = 1;

/* Set if full-page illustrations don't have page numbers */
static int unnumbered_illustrations = 0; 

FILE *get_outfile()
{
  return outfile;
}

void style_sheet()
{
  fwprintf(outfile, L"<style type=\"text/css\">\n");
#if 0
  fwprintf(outfile, L"/*<![CDATA[  XML blockout */\n");
  fwprintf(outfile, L"<!--\n");
#endif

  fwprintf(outfile, L"  p {\n");
  fwprintf(outfile, L"    margin-top: .75em;\n");
  fwprintf(outfile, L"    text-align: justify;\n");
  fwprintf(outfile, L"    margin-bottom: .75em;\n");
  fwprintf(outfile, L"    }\n\n");

  fwprintf(outfile, L"  h1,h2,h3,h4,h5,h6 {\n");
  fwprintf(outfile, L"    text-align: center;\n");
  fwprintf(outfile, L"    clear: both;\n");
  fwprintf(outfile, L"    }\n\n");

  fwprintf(outfile, L"  body {\n");
  fwprintf(outfile, L"    margin-left: 15%%;\n");
  fwprintf(outfile, L"    margin-right: 15%%;\n");
  fwprintf(outfile, L"    }\n\n");

  fwprintf(outfile, L"  h2 {margin-top: 2em;}\n\n");

  fwprintf(outfile, L"  .h2a {\n");
  fwprintf(outfile, L"    text-align: center;\n");
  fwprintf(outfile, L"    font-weight: bold;\n");
  fwprintf(outfile, L"    margin-bottom: 1.5em;\n");
  fwprintf(outfile, L"    }\n\n");

  fwprintf(outfile, L"  .rmn { left: 92%%; position: absolute; text-align: right;}\n\n");

  fwprintf(outfile, L"  .pagenum { left: 92%%; position: absolute; text-align: right; font-weight: normal; font-size: small; color: #808080;}\n\n");

  fwprintf(outfile, L"  .fnref {vertical-align: 0.25em; font-size: 0.8em; text-decoration: none;}\n\n");

  fwprintf(outfile, L"  .footnote {margin-left: 2em; margin-right: 2em; font-size: small;}\n\n");

  fwprintf(outfile, L"  .sidenote {float: right; border: solid 1px; padding-right: 0.5em; padding-left: 0.5em; margin-left: 0.5em; width: 25%%}\n\n");

  fwprintf(outfile, L"  .nowrap {margin-left: 2em;}\n\n");

  fwprintf(outfile, L"  .figure {margin-top: 2em; text-align: center;}\n\n");

  fwprintf(outfile, L"  .caption {text-align: center}\n\n");

  fwprintf(outfile, L"  .indented {margin-left: 2em}\n\n");
  fwprintf(outfile, L"  .indented2 {margin-left: 6em}\n\n");
  fwprintf(outfile, L"  .indented3 {margin-left: 8em}\n\n");
  fwprintf(outfile, L"  .indented4 {margin-left: 10em}\n\n");
  fwprintf(outfile, L"  .indented5 {margin-left: 12em}\n\n");
  fwprintf(outfile, L"  .indented6 {margin-left: 14em}\n\n");
  fwprintf(outfile, L"  .indented7 {margin-left: 16em}\n\n");

  fwprintf(outfile, L"  .smcap {font-variant: small-caps;}\n\n");

  fwprintf(outfile, L"  .allsmcap {font-size: smaller;}\n\n");

  fwprintf(outfile, L"  .fraktur {font-style: italic;}\n\n");

  fwprintf(outfile, L"  .gesperrt {font-variant: small-caps;}\n\n");

  fwprintf(outfile, L"  .underline {text-decoration: underline;}\n\n");

  /* size1 and size2 are special project-specific markup (for John Dee) */
  fwprintf(outfile, L"  .size1 {font-weight: bold;}\n\n");

  fwprintf(outfile, L"  .size2 {font-weight: bold; font-size: large;}\n\n");

  fwprintf(outfile, L"  .comment {color: red}\n\n");

  fwprintf(outfile, L"  .handwriting {font-style: italic}\n\n");

#if 0
  fwprintf(outfile, L"// -->\n");
  fwprintf(outfile, L"/* XML end  ]]>*/\n");
#endif
  fwprintf(outfile, L"</style>\n");
}

static int page = 0;

int get_pagenumber()
{
  return page;
}

void report_error(wchar_t *msg, wchar_t *str)
{
  fwprintf(stderr, L"Page %d: %ls\n", page, msg);
  fwprintf(stderr, L"%ls\n", str);
}


void found_illustration()
{
  if (unnumbered_illustrations)
    page -= 2;
}

void output_pagenumber(int number_pages, int page, int front_pages, int preface_pages, int volume_pages, int page_offset)
{
  if (number_pages)
  {
    if ((volume_pages == 0) || (page <= volume_pages))
    {
      if (page <= front_pages)
      {
      }
      else if (page <= front_pages+preface_pages)
        fwprintf(outfile, L"<a name=\"preface%d\"></a><span class=\"pagenum\">[Pg&nbsp;%d]</span>\n", page-front_pages, page-front_pages);
      else
        fwprintf(outfile, L"<a name=\"page%d\"></a><span class=\"pagenum\">[Pg&nbsp;%d]</span>\n", page-preface_pages-front_pages+page_offset, page-preface_pages-front_pages+page_offset);
    }
    else
    {
      if (page-volume_pages <= front_pages)
      {
      }
      else if (page-volume_pages <= front_pages+preface_pages)
        fwprintf(outfile, L"<a name=\"preface_2_%d\"></a><span class=\"pagenum\">[Pg&nbsp;%d]</span>\n", page-volume_pages-front_pages, page-volume_pages-front_pages);
      else
        fwprintf(outfile, L"<a name=\"page_2_%d\"></a><span class=\"pagenum\">[Pg&nbsp;%d]</span>\n", page-volume_pages-preface_pages-front_pages, page-volume_pages-preface_pages-front_pages);
    }
  }
  else
    fwprintf(outfile, L"<!-- Page %d -->\n", page);
}

void finish_paragraph()
{
  switch (par_type)
  {
    case PAR_TYPE_NORMAL:
      fwprintf(outfile, L"</p>\n");
      break;
    case PAR_TYPE_SECTION:
      fwprintf(outfile, L"</h3>\n");
      break;
    case PAR_TYPE_CHAPTER_A:
      fwprintf(outfile, L"</p>\n");
      break;
    case PAR_TYPE_CHAPTER:
      fwprintf(outfile, L"</h2>\n");
      break;
    case PAR_TYPE_RULE:
      break;
    default:
      break;
  }
}

void output_header()
{
  fwprintf(outfile, L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");

  fwprintf(outfile, L"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n");
  fwprintf(outfile, L"<head>\n");
  fwprintf(outfile, L"<title>Title goes here</title>\n");
  fwprintf(outfile, L"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />");
  style_sheet();
  fwprintf(outfile, L"</head>\n");
  fwprintf(outfile, L"<body>\n");
}

void start_paragraph()
{
  switch (blank_lines)
  {
    case 0:
      break;
    case 1:
      if ((par_type == PAR_TYPE_CHAPTER)||(par_type == PAR_TYPE_CHAPTER_A))
      {
        fwprintf(outfile, L"<p class=\"h2a\">\n");
        par_type = PAR_TYPE_CHAPTER_A;
      }
      else
      {
        if (wcsncmp(buff, L"[Illustration", 13) == 0)
        {
          fwprintf(outfile, L"<p class=\"figure\">\n");
          par_type = PAR_TYPE_NORMAL;
        }
        else if (wcscmp(buff, L"<tb>") == 0)
        {
          fwprintf(outfile, L"<hr />");
          par_type = PAR_TYPE_RULE;
        }
        else if (poetry_mode)
        {
          fwprintf(outfile, L"<p class=\"nowrap\">\n");
          par_type = PAR_TYPE_NORMAL;
        }
        else
        {
          fwprintf(outfile, L"<p>\n");
          par_type = PAR_TYPE_NORMAL;
        }
      }
      break;
    case 2:
      /* Two blank lines after a normal paragraph denotes a section heading.
       * Two blank lines after a chapter title indicates the end of the
       * chapter title.
       */
      if ((par_type == PAR_TYPE_CHAPTER)||(par_type == PAR_TYPE_CHAPTER_A))
      {
        if ((wcsncmp(buff, L"/*", 2) == 0) || poetry_mode)
          fwprintf(outfile, L"<p class=\"nowrap\">\n");
        else
          fwprintf(outfile, L"<p>\n");
        par_type = PAR_TYPE_NORMAL;
      }
      else
      {
        fwprintf(outfile, L"<h3>\n");
        par_type = PAR_TYPE_SECTION;
        section++;
        if (number_sections)
          fwprintf(outfile, L"<a name=\"section%d_%d\"></a>\n",
            chapter, section);
      }
      break;
    case 4:
      fwprintf(outfile, L"<h2>\n");
      chapter++;
      section = 1; 
      /* Start section numbering at 2, because the ambiguous syntax means
       *  that the first section in a chapter will often be missed, and
       * will have to be added manually.
       */
      fwprintf(outfile,
        L"<a name=\"chapter%d\"></a>\n",
        chapter-chapter_offset);
      par_type = PAR_TYPE_CHAPTER;
      break;
    default:
      fwprintf(outfile, L"<b>Unexpected number of blank lines (%d)!</b>\n", blank_lines);
      fwprintf(outfile, L"<p>\n");
      par_type = PAR_TYPE_NORMAL;
      break;
  }
}

void end_document()
{
  switch (par_type)
  {
    case PAR_TYPE_NORMAL:
      fwprintf(outfile, L"</p>\n");
      break;
    case PAR_TYPE_CHAPTER:
      fwprintf(outfile, L"</h2>\n");
      break;
    case PAR_TYPE_CHAPTER_A:
      fwprintf(outfile, L"</h2>\n");
      break;
    case PAR_TYPE_SECTION:
      fwprintf(outfile, L"</h3>\n");
      break;
  }
  if (quote_mode == 2)
    fwprintf(outfile, L"</blockquote>\n");

  fwprintf(outfile, L"</body>\n");
  fwprintf(outfile, L"</html>\n");
}

void open_poetry()
{
  switch (poetry_mode)
  {
    case 0:
      poetry_mode = 1;
      break;
    case 1:
      fwprintf(stderr, L"Poetry markers already open on page %d.\n", page);
      break;
    case 2:
      poetry_mode = 1;
      break;
  }
}

void open_quotation()
{
  switch (quote_mode)
  {
    case 0:
      quote_mode = 1;
      fwprintf(outfile, L"<blockquote>\n");
      break;
    case 1:
      fwprintf(stderr, L"Blockquote markers already open on page %d.\n", page);
      break;
    case 2:
      quote_mode = 1;
      break;
  }
}

void close_poetry()
{
  switch (poetry_mode)
  {
    case 0:
      fwprintf(stderr, L"Poetry markers not open on page %d.\n", page);
      break;
    case 1:
      poetry_mode = 0;
      break;
    case 2:
      fwprintf(stderr, L"Poetry markers not open on page %d.\n", page);
      break;
  }
}

void close_quotation()
{
  switch (quote_mode)
  {
    case 0:
      fwprintf(stderr, L"Blockquote markers not open on page %d.\n", page);
      break;
    case 1:
      quote_mode = 2;
   /*   wprintf(L"</blockquote>\n"); */
      break;
    case 2:
      fwprintf(stderr, L"Blockquote markers not open on page %d.\n", page);
      break;
  }
}

void check_open_footnote()
{
  if ((wcsncmp(buff, L"[Footnote:", 10) == 0)
    || (wcsncmp(buff, L"*[Footnote:", 11) == 0)
    || (wcsncmp(buff, L"[Footnote ", 10) == 0)
    || (wcsncmp(buff, L"*[Footnote ", 11) == 0))
  {
    fwprintf(outfile, L"<div class=\"footnote\">\n");
    footnote_mode = 1;
  }

  if ((wcsncmp(buff, L"[Sidenote:", 10) == 0)
    || (wcsncmp(buff, L"*[Sidenote:", 11) == 0))
  {
    fwprintf(outfile, L"<div class=\"sidenote\">\n");
    sidenote_mode = 1;
  }
}

void check_close_footnote()
{
  if (footnote_mode && (get_footnote_mode() == 0))
  {
    fwprintf(outfile, L"</div>\n");
    footnote_mode = 0;
  }

  if (sidenote_mode && (get_sidenote_mode() == 0))
  {
    fwprintf(outfile, L"</div>\n");
    sidenote_mode = 0;
  }
}

int main(int argc, char **argv)
{
int len;
int count = 0;
int newpage = 0;
int i;
int number_pages = 0;
int preface_pages = 0;
int volume_pages = 0;
int front_pages = 0;
int page_offset = 0;
int drama_brackets = 0;
int c;
int unicode_fopen = 0; /* For Windows: set if need to pass a Unicode mode to fopen */
int para_open = 0;

  /* Need to set the locale before can print wide characters to stdout */
  setlocale(LC_ALL, getenv("LANG"));

  outfile = stdout;

  while ((c = getopt(argc, argv, "nyo:uC:DF:O:P:UV:")) > -1)
  {
    switch (c)
    {
      case 'n':
        number_pages = 1;
        break;
      case 'o':
        if (unicode_fopen)
          outfile = fopen(optarg, "w, ccs=UTF-8");
        else
          outfile = fopen(optarg, "w");
        break;
      case 'u':
        unicode_fopen = 1;
        break;
      case 'y':
        set_yogh_mode(1);
        break;
      case 'C':
        chapter_offset = atoi(optarg);
        break;
      case 'D':
        drama_brackets = 1;
        set_drama_brackets(drama_brackets);
        break;
      case 'F':
        front_pages = atoi(optarg);
        break;
      case 'O':
        page_offset = atoi(optarg);
        break;
      case 'P':
        preface_pages = atoi(optarg);
        break;
      case 'U':
         unnumbered_illustrations = 1;
         break;
      case 'V':
         volume_pages = atoi(optarg);
         break;
    }
  }

  translit_init();

  output_header();

  while (fgetws(buff, sizeof(buff), stdin) > 0)
  {
    count++;
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

    /* Characters from 0x80 to 0x9f are almost certainly due to
     * incorrect translation between character sets.
     */

    for (i=0;i<len;i++)
      if ((buff[i]>=0x80) && (buff[i]<0xa0))
        fwprintf(stderr, L"Unexpected control character: 0x%x\n", buff[i]);

    /*
     * A blank line denotes a paragraph break
     * 2 blank lines denote a section break
     * 4 blank lines denote a chapter break
     */

    if (len == 0)
    {
      blank_lines++;
    }
    else if (wcsncmp(buff, L"-----", 5) == 0)
    {
      page++;
      newpage = 1;
      blank_lines = 0;  /* ignore any blank lines at end of previous page */
      if (poetry_mode == 1)
      {
        fwprintf(stderr, L"Poetry markers not closed at end of page %d.\n", page);
        poetry_mode = 0;
      }
      if (quote_mode == 1)
      {
        fwprintf(stderr, L"Block quotation markers not closed at end of page %d.\n", page);
        fwprintf(outfile, L"</blockquote>\n");
        quote_mode = 0;
      }
      output_pagenumber(number_pages, page, front_pages, preface_pages, volume_pages, page_offset);
    }
    else if (wcscmp(buff, L"[Blank Page]") == 0)
    {
    }
    else
    {
      /* Finish off the previous paragraph */

      if (blank_lines > 0)
      {
        if (para_open)
        {
          if (drama_brackets)
            finish_drama_bracket();
          finish_paragraph();
          para_open = 0;
        }
        if (quote_mode == 2)
        {
          fwprintf(outfile, L"</blockquote>\n");
          quote_mode = 0;
        }
        check_close_footnote();
      }

      if (wcscmp(buff, L"/*") == 0)
      {
        open_poetry();
      }
      else if (wcscmp(buff, L"/#") == 0)
      {
        open_quotation();
      }
      else if (wcscmp(buff, L"*/") == 0)
      {
        close_poetry();
      }
      else if (wcscmp(buff, L"#/") == 0)
      {
        close_quotation();
      }
      else
      {
        if (blank_lines > 0)
        {
          check_open_footnote();
          start_paragraph();
          blank_lines = 0;
          para_open = 1;
        }

        if (poetry_mode)
          write_poetry_line(outfile, buff);
        else
          write_line(outfile, buff);
        fwprintf(outfile, L"\n");
      }
    }
    /*
     * Page-break markers are not part of the transcribed text
     * NB: Paragraphs can continue across a page-break
     */


    if (wcsncmp(buff, L"-----", 5) == 0)
    {
      flush_tags(outfile);
    }

  }

  /*
   ( Enf of document.
   * Close any tags that are still open.
   */

  end_document();

  return 0;
}
