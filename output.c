/*
 * output.c
 *
 * Reformat Distributed Proofreaders files for easy viewing
 */

#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <stdlib.h>

#include "dptools.h"
#include "entity.h"

/* In "yogh mode", [3] denotes LATIN SMALL LETTER YOGH, not a footnote */
static int yogh_mode = 0; 

static int long_s_mode = 0;

static int drama_brackets = 1;

static int greek_mode = 0; /* Greek transliteration mode, entered by [Greek: */

static int sup_mode = 0; /* Superscript mode */

static int sub_mode = 0; /* subscript mode */

static int footnote_mode = 0;

static int sidenote_mode = 0;

static int use_html_entities = 0;

void set_yogh_mode(int val)
{
  fwprintf(stderr, L"set_yogh_mode\n");
  yogh_mode = 1;
}

void set_drama_brackets(int val)
{
  drama_brackets = val;
}

int get_footnote_mode()
{
  return footnote_mode;
}

int get_sidenote_mode()
{
  return sidenote_mode;
}

#define TAG_ITALIC 1
#define TAG_BOLD 2
#define TAG_SC 3
#define TAG_GREEK 4
#define TAG_COMMENT 5
#define TAG_SIDENOTE 6
#define TAG_FOOTNOTE 7
#define TAG_SYMBOL 8
#define TAG_UNKNOWN 9
#define TAG_ILLUSTRATION 10
#define TAG_SIZE 11
#define TAG_SUPERSCRIPT 12
#define TAG_UNDERLINE 13
#define TAG_HANDWRITING 14
#define TAG_ASC 15
#define TAG_GESPERRT 16
#define TAG_FRAKTUR 17 /* MArks text that is NOT in Fraktur */
#define TAG_SUBSCRIPT 18
#define TAG_SUPERSCRIPT1 19

#define TAG_STACK_SIZE 50

wchar_t *tag_names[] = {
  L"null",
  L"italic",
  L"bold",
  L"small caps",
  L"greek",
  L"comment",
  L"sidenote",
  L"footnote",
  L"symbol",
  L"unknown",
  L"illustration",
  L"size",
  L"superscript",
  L"underline",
  L"handwriting",
  L"allsmallcaps",
  L"gesperrt",
  L"fraktur",
  L"subscript",
  L"superscript1",
  L"illustration"
};

int tag_stack[TAG_STACK_SIZE];
int tags_on_stack = 0;

int push_tag(int tag)
{
 /* fwprintf(stderr, L"push %s\n", tag_names[tag]);  */
  if (tags_on_stack >= TAG_STACK_SIZE)
    return -1;

  tag_stack[tags_on_stack] = tag;
  tags_on_stack++;

  return 0;
}


int pop_tag()
{
  /* fwprintf(stderr, L"pop\n");  */
  if (tags_on_stack == 0)
    return 0;

  tags_on_stack--;
  return tag_stack[tags_on_stack];
}

int top_tag()
{
  if (tags_on_stack == 0)
    return 0;

  return tag_stack[tags_on_stack-1];
}

void flush_tags(FILE *outfile)
{
  /* In drama formatting, it's OK to have one square bracket that isn't
   * clsoed by the end of the page.
   */
  if (drama_brackets && (top_tag() == TAG_UNKNOWN))
    pop_tag();

  if (tags_on_stack > 0)
    fwprintf(stderr, L"Tags not closed at end of page %d.\n", get_pagenumber());

  while (tags_on_stack > 0)
  {
    tags_on_stack--;
    switch (tag_stack[tags_on_stack])
    {
      case TAG_ITALIC:
        fwprintf(outfile, L"</i>");
        break;
      case TAG_BOLD:
        fwprintf(outfile, L"</b>");
        break;
      case TAG_SC:
        fwprintf(outfile, L"</span>");
        break;
      case TAG_GREEK:
        greek_mode = 0;
        break;
      case TAG_COMMENT:
	fwprintf(outfile, L"</span>");
        break;
      case TAG_SIDENOTE:
        fwprintf(outfile, L"</span>");
        break;
      case TAG_SUPERSCRIPT:
      case TAG_SUPERSCRIPT1:
        fwprintf(outfile, L"</sup>");
        break;
      case TAG_UNDERLINE:
        fwprintf(outfile, L"</span>");
        break;
      case TAG_HANDWRITING:
        fwprintf(outfile, L"</span>");
        break;
      case TAG_GESPERRT:
        fwprintf(outfile, L"</span>");
        break;
      case TAG_FRAKTUR:
        fwprintf(outfile, L"</span>");
        break;
      case TAG_SUBSCRIPT:
        fwprintf(outfile, L"</sub>");
        break;
      case TAG_ILLUSTRATION:
        break;
      default:
        fwprintf(stderr, L"Unexpected tag on stack: %ls\n",
          tag_names[tag_stack[tags_on_stack]]);
        break;
     }
  }
}

int is_footnote(wchar_t *str, int *val, int *lenp)
{
int len = 0;
wchar_t buff[20];

  len++;
  str++;

  if (*str == ']')
    return 0;

  if (*str == '\0')
    return 0;

  while (*str != ']')
  {
    if (*str == '\0')
      return 0;
    if (iswdigit(*str) == 0)
      return 0;
    if (len > 20)
      return 0; 
    buff[len-1] = *str;
    len++;
    str++;
  }
  buff[len-1] = '\0';
  *val = wcstol(buff, NULL, 10);
  len++; /* The closing square bracket is included in the length */
  *lenp = len;
  return 1;
}

static int footnote_section = 0;
static int footnote_counter = 0;

void write_line(FILE *outfile, wchar_t *str)
{
  wchar_t *cp;
  int footnote_num;
  int len;
  struct entity *e;

  cp = str;

  while (*cp != L'\0')
  {
    switch (*cp)
    {
      /*
       * These three characters are special in HTML, and must be escaped.
       */
      case '"':
        fwprintf(outfile, L"&quot;");
        cp++;
        break;
      case '&':
        fwprintf(outfile, L"&amp;");
        cp++;
        break;
      case '>':
        fwprintf(outfile, L"&gt;");
        cp++;
        break;
      /*
       * Other characters are valid in HTML, but can be escaped if we
       * want the file to be ISO LATIN 1 only.
       */
      case 0x2018:
        if (use_html_entities)
	  fwprintf(outfile, L"&lsquo;");
	else
          fputwc(*cp, outfile);
        cp++;
        break;
      case 0x2019:
	if (use_html_entities)
          fwprintf(outfile, L"&rsquo;");
	else
          fputwc(*cp, outfile);
        cp++;
        break;
      case 0x201c:
	if (use_html_entities)
          fwprintf(outfile, L"&ldquo;");
	else
          fputwc(*cp, outfile);
        cp++;
        break;
      case 0x201d:
	if (use_html_entities)
          fwprintf(outfile, L"&rdquo;");
	else
	  fputwc(*cp, outfile);
        cp++;
        break;
      case '^':
        sup_mode = 1;
        fwprintf(outfile, L"<sup>");
        if (cp[1] == '{')
        {
          cp += 2;
          push_tag(TAG_SUPERSCRIPT);
        }
        else
        {
          cp++;
          push_tag(TAG_SUPERSCRIPT1);
        }
        break;
      case '_':
        if (cp[1] == '{')
        {
          cp += 2;
          push_tag(TAG_SUBSCRIPT);
          sub_mode = 1;
          fwprintf(outfile, L"<sub>");
        }
        else if (greek_mode)
        {
          write_greek_char(outfile, *cp);
          cp++;
        }
        else
        {
          fputwc(*cp, outfile);
          cp++;
        }
        break;
      case '<':
        if (wcsncmp(cp, L"<i>", 3) == 0)
        {
          fwprintf(outfile, L"<i>");
          cp += 3;
          push_tag(TAG_ITALIC);
        }
        else if (wcsncmp(cp, L"<b>", 3) == 0)
        {
          fwprintf(outfile, L"<b>");
          cp += 3;
          push_tag(TAG_BOLD);
        }
        else if (wcsncmp(cp, L"<g>", 3) == 0)
        {
          fwprintf(outfile, L"<span class=\"gesperrt\">");
          cp += 3;
          push_tag(TAG_GESPERRT);
        }
        else if (wcsncmp(cp, L"<f>", 3) == 0)
        {
          fwprintf(outfile, L"<span class=\"fraktur\">");
          cp += 3;
          push_tag(TAG_FRAKTUR);
        }
        else if (wcsncmp(cp, L"<sc>", 4) == 0)
        {
          fwprintf(outfile, L"<span class=\"smcap\">");
          cp += 4;
          push_tag(TAG_SC);
        }
        else if (wcsncmp(cp, L"<tb>", 4) == 0)
        {
          cp += 4; /* <tb> is handled in gutf.c, not here. */
        }
        else if (wcsncmp(cp, L"<asc>", 5) == 0)
        {
          fwprintf(outfile, L"<span class=\"allsmcap\">");
          cp += 5;
          push_tag(TAG_ASC);
        }
        else if (wcsncmp(cp, L"<size 1>", 8) == 0)
        {
          fwprintf(outfile, L"<span class=\"size1\">");
          cp += 8;
          push_tag(TAG_SIZE);
        }
        else if (wcsncmp(cp, L"<size 2>", 8) == 0)
        {
          fwprintf(outfile, L"<span class=\"size2\">");
          cp += 8;
          push_tag(TAG_SIZE);
        }
        else if (wcsncmp(cp, L"<u>", 3) == 0)
        {
          fwprintf(outfile, L"<span class=\"underline\">");
          cp += 3;
          push_tag(TAG_UNDERLINE);
        }
        else if (wcsncmp(cp, L"</i>", 4) == 0)
        {
          cp += 4;
          if (top_tag() != TAG_ITALIC)
            fwprintf(stderr, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</i>");
            pop_tag();
          }
        }
        else if (wcsncmp(cp, L"</b>", 4) == 0)
        {
          cp += 4;
          if (top_tag() != TAG_BOLD)
            fwprintf(stdout, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</b>");
            pop_tag();
          }
        }
        else if (wcsncmp(cp, L"</sc>", 5) == 0)
        {
          cp += 5;
          if (top_tag() != TAG_SC)
            fwprintf(stdout, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</span>");
            pop_tag();
          }
        }
        else if (wcsncmp(cp, L"</f>", 4) == 0)
        {
          cp += 4;
          if (top_tag() != TAG_FRAKTUR)
            fwprintf(stdout, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</span>");
            pop_tag();
          }
        }
        else if (wcsncmp(cp, L"</g>", 4) == 0)
        {
          cp += 4;
          if (top_tag() != TAG_GESPERRT)
            fwprintf(stdout, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</span>");
            pop_tag();
          }
        }
        else if (wcsncmp(cp, L"</asc>", 6) == 0)
        {
          cp += 6;
          if (top_tag() != TAG_ASC)
            fwprintf(stdout, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</span>");
            pop_tag();
          }
        }
        else if (wcsncmp(cp, L"</size>", 7) == 0) /* <size> is a special for John Dee */
        {
          cp += 7;
          if (top_tag() != TAG_SIZE)
            fwprintf(stdout, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</span>");
            pop_tag();
          }
        }
        else if (wcsncmp(cp, L"</u>", 4) == 0)
        {
          cp += 4;
          if (top_tag() != TAG_UNDERLINE)
            fwprintf(stderr, L"Tags don't match\n");
          else
          {
            fwprintf(outfile, L"</span>");
            pop_tag();
          }
        }
        else
        {
          fwprintf(outfile, L"&lt;");
          cp++;
        }
        break;
      case '-':
        if (wcsncmp(cp, L"----", 4) == 0)
        {
          fwprintf(outfile, L"&mdash;&mdash;"); /* really, a single long dash */
          cp += 4;
        }
        else if (wcsncmp(cp, L"--", 2) == 0)
        {
          fwprintf(outfile, L"&mdash;");
          cp += 2;
        }
        else
        {
          fwprintf(outfile, L"-");
          cp++;
        }
        break;
      case '[':
        if ((wcsncmp(cp, L"[f]", 3) == 0) && long_s_mode) /* non-standard addition: long s */
        {
           fwprintf(outfile, L"s");
           cp += 3;
        }
        else if (yogh_mode && (wcsncmp(cp, L"[3]", 3) == 0))
        {
          fwprintf(outfile, L"&#x021d;");
          cp += 3;
        }
        else if (is_footnote(cp, &footnote_num, &len))
        {
          if (footnote_num == 1)
            footnote_section++;
            footnote_counter = 0;
          /* Each time the footnote numbering restarts from 1, increment
           * footnote_section, so that each footnote gets a unique label.
           */
          fwprintf(outfile, L"<a name=\"ref_%d_%d\"></a><a href=\"#footnote_%d_%d\" class=\"fnref\">[%d]</a>", footnote_section, footnote_num, footnote_section, footnote_num, footnote_num);
          cp += len;
        }
        else if ((cp[1] != '\0') && (cp[2] == ']')
                 && (cp[1] >= 'A') && (cp[1] <= 'Z'))
        {
          fwprintf(outfile, L"<span class=\"fnref\">[%lc]</span>", cp[1]);
          cp += 3;
        }
        else if (wcsncmp(cp, L"[']", 3) == 0)
        {
          flush_greek(outfile);
          fputwc(0x374, outfile);
          cp += 3;
        }
        else if (wcsncmp(cp, L"[st]", 4) == 0)
        {
          flush_greek(outfile);
          fputwc(0x3db, outfile);
          cp += 4;
        }
        else if (wcsncmp(cp, L"[ST]", 4) == 0)
        {
          flush_greek(outfile);
          fputwc(0x3da, outfile);
          cp += 4;
        }
        else if (e = find_entity(cp, &len))
        {
          if (use_html_entities && e->html)
            fwprintf(outfile, L"%ls", e->html);
          else
            fwprintf(outfile, L"&#x%04x;", e->unicode);
          cp += len;
        }
        else if (wcsncmp(cp, L"[3*]", 4) == 0)
        {
          fwprintf(outfile, L"&#x021c;");
          cp += 4;
        }
        else if (wcsncmp(cp, L"[Blank Page]", 12) == 0)
        {
          cp += 12;
        }
        else if (wcsncmp(cp, L"[Illustration]", 14) == 0)
        {
          cp += 14;
          fwprintf(outfile, L"<img src=\"images/missing.jpg\" alt=\"Missing image\">\n");
          found_illustration();
        }
        else if (wcsncmp(cp, L"[Illustration:", 14) == 0)
        {
          cp += 14;
          while (*cp == ' ')
            cp++;
          fwprintf(outfile, L"<img src=\"images/missing.jpg\" alt=\"Missing image\">\n");
          fwprintf(outfile, L"</p>\n");
          fwprintf(outfile, L"<p class=\"caption\">\n");
          push_tag(TAG_ILLUSTRATION);
          found_illustration();
        }
        else if (wcsncmp(cp, L"[*]", 3) == 0)
        {
          fwprintf(outfile, L"<span class=\"fnref\">*</span>");
          cp += 3;
        }
        else if (wcsncmp(cp, L"[Greek:", 7) == 0)
        {
          push_tag(TAG_GREEK);
          greek_mode = 1;
          cp += 7;
          while (*cp == ' ')
            cp++;
        }
        else if (wcsncmp(cp, L"[Symbol:", 8) == 0)
        {
          push_tag(TAG_SYMBOL);
          fwprintf(outfile, L"[Symbol:");
          cp += 8;
        }
        else if (wcsncmp(cp, L"[Sidenote:", 10) == 0)
        {
          push_tag(TAG_SIDENOTE);
          sidenote_mode = 1;
          cp += 10;
          while (*cp == ' ')
            cp++;
        } 
        else if (wcsncmp(cp, L"[Footnote:", 10) == 0)
        {
          push_tag(TAG_FOOTNOTE);
          footnote_mode = 1;
          /* The "footnote" class is handled in gutf.c, not here,
           * paragraphs are nested inside footnotes.
           */
          cp += 10;
          while (*cp == ' ')
            cp++;
          footnote_counter++;
          fwprintf(outfile,
            L"<a name=\"footnote_%d_%d\"></a>",
            footnote_section,
            footnote_counter);
        }
        else if (wcsncmp(cp, L"[Footnote", 9) == 0)
        {
          push_tag(TAG_FOOTNOTE);
          footnote_mode = 1;
          cp += 9;
          while (*cp == ' ')
            cp++;
          footnote_counter++;
          fwprintf(outfile,
            L"<a name=\"footnote_%d_%d\"></a>",
            footnote_section,
            footnote_counter);
          fwprintf(outfile, L"<a href=\"#ref_%d_%d\">",
            footnote_section,
            footnote_counter);
          while ((*cp != '\0') && (*cp != ':'))
          {
            fputwc(*cp, outfile);
            cp++;
          }
          fwprintf(outfile, L"</a>"); 
        }
        else if (wcsncmp(cp, L"[**", 3) == 0)
        {
          push_tag(TAG_COMMENT);
          fwprintf(outfile, L"<span class=\"comment\">[** ");
          cp += 3;
        }
        else if (wcsncmp(cp, L"[HW:", 4) == 0)
        {
          push_tag(TAG_HANDWRITING);
          fwprintf(outfile, L"<span class=\"handwriting\">");
          cp += 4;
          while (*cp == ' ')
            cp++;
        }
        else
        {
          fwprintf(outfile, L"%lc", *cp);
          push_tag(TAG_UNKNOWN);
          if (!drama_brackets)
            fwprintf(stderr, L"Unrecognized sequence: %ls\n", cp);
          cp++;
        }
        break;
      case ']':
        if (sup_mode)
        {
          fwprintf(outfile, L"</sup>");
          sup_mode = 0;
          pop_tag();
        }
        switch (pop_tag())
        {
          case TAG_ITALIC:
          case TAG_BOLD:
          case TAG_SC:
            fwprintf(stderr, L"Tags don't match\n");
            break;
          case TAG_COMMENT:
            fwprintf(outfile, L"]</span>");
            break;
          case TAG_SIDENOTE:
            sidenote_mode = 0;
            break;
          case TAG_FOOTNOTE:
            /* The footnote class is handled in gutf.c, not here. */
            footnote_mode = 0;
            break;
          case TAG_HANDWRITING:
            fwprintf(outfile, L"</span>");
            break;
          case TAG_GREEK:
            flush_greek(outfile);
            greek_mode = 0;
            break;
          case TAG_ILLUSTRATION:
            break;
          case TAG_UNKNOWN:
          default:
            fwprintf(outfile, L"%lc", *cp);
            break;
        }
        cp++;
        break;
      case '}':
        if (sup_mode && (top_tag() == TAG_SUPERSCRIPT))
        {
          fwprintf(outfile, L"</sup>");
          pop_tag();
          sup_mode = 0;
        }
        else if (sub_mode && (top_tag() == TAG_SUBSCRIPT))
        {
          fwprintf(outfile, L"</sub>");
          pop_tag();
          sub_mode = 0;
        }
        else if (greek_mode)
          write_greek_char(outfile, *cp);
        else
          fputwc(*cp, outfile);
        cp++;
        break;
      default:
        /* Punctuation or white space ends a superscript */
        if (sup_mode && (top_tag() == TAG_SUPERSCRIPT1) &&
          ((*cp == ' ') || (*cp == '.') || (*cp == ',')
          || (*cp == '?') || (*cp == '!')))
        {
          fwprintf(outfile, L"</sup>");
          sup_mode = 0;
          pop_tag();
        }

        if (greek_mode)
          write_greek_char(outfile, *cp);
        else
          fwprintf(outfile, L"%lc", *cp);
        cp++;
      }
  }
  if (sup_mode)
  {
    fwprintf(outfile, L"</sup>");
    if (top_tag() == TAG_SUPERSCRIPT1)
      pop_tag();
    else
      fwprintf(stderr, L"Tags don't match\n");

    sup_mode = 0;
  }
}

void write_poetry_line(FILE *outfile, wchar_t *str)
{
  wchar_t lbuff[1024];
  int spaces;
  int len;
  int i;
  wchar_t *cp;
  wchar_t *right;
  wchar_t *left;

  spaces = 0;
  cp = str;
  while (*cp == L' ')
  {
    spaces++;
    cp++;
  }

  right = wcsstr(cp, L"      ");
  if (right)
  {
    len = right-cp;
    if (len >= sizeof(lbuff))
    {
      fwprintf(stderr, L"Left part of line too long\n");
      len = sizeof(lbuff)-1;
    }
    wcsncpy(lbuff, cp, len);
    lbuff[len] = L'\0';
    left = lbuff;

    while (*right == L' ')
      right++;
  }
  else
  {
    left = cp;
  }

  for (i=0;i<spaces;i++)
    fwprintf(outfile, L"&nbsp;&nbsp;");
  write_line(outfile, left);

  if (right)
  {
    /* NB: Put in a space between the end of the first part of the line
     * and the line number.
     */
    fwprintf(outfile, L" <span class=\"rmn\">");
    write_line(outfile, right);
    fwprintf(outfile, L"</span>");
  }

  /* In drama, stage directions start with an opening square bracket
   * that has no matching close bracket.
   */
  if (drama_brackets && (top_tag() == TAG_UNKNOWN))
  {
    pop_tag();
  }
  fwprintf(outfile, L"<br>");
}

void finish_drama_bracket()
{
  if (top_tag() == TAG_UNKNOWN)
  {
    pop_tag();
  }
}
