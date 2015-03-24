/*****************************************************
**     _________       __                           **
**     \_   ___ \_____|__| _____  ________  ___     **
**      /    \  \/_  __ \ |/     \/  ___/_ \/   \   **
**      \     \___|  | \/ |  | |  \___ \  / ) |  \  **
**       \______  /__| |__|__|_|  /____ \__/__|  /  **
**         ____\/____ _        \/ ___ \/      \/    **
**         \______   \ |_____  __| _/___            **
**          |    |  _/ |\__  \/ __ | __ \           **
**          |    |   \ |_/ __ \  / | ___/_          **
**          |_____  /__/____  /_  /___  /           **
**               \/Antipode\/  \/    \/             **
******************************************************
**         Crimson Blade Codebase (CbC)             **
**     (c) 2000-2002 John Bellone (Noplex)          **
**           Coders: Noplex, Krowe                  **
**        http://www.crimsonblade.org               **
*****************************************************/

/*
 * File: news.h
 * Name: Extended News (v2.81)
 * Author: John 'Noplex' Bellone (john.bellone@flipsidesoftware.com)
 * Terms:
 * If this file is to be re-disributed; you must send an email
 * to the author. All headers above the #include calls must be
 * kept intact.
 * Description:
 * This is the extended news module; it allows for news to be
 * posted in note-like format; and bringing you into a editbuffer
 * instead of one-line posts (like Elder Chronicles). It also
 * allows support for online HTML output for news to be automatically
 * generated and included via a PHP; SSL; or a TXT include.
 */

#define NEWS_FILE "news.dat"
#define NEWS_INCLUDE_FILE "news.txt"
#define NEWS_TOP "\n\r"
#define NEWS_HEADER "\n\r"
#define NEWS_HEADER_ALL "&g( &W#&g)                          (&WSubject&g)\n\r"
#define NEWS_HEADER_READ "&g( &W#&g)                          (&WSubject&g)\n\r"
#define NEWS_VIEW               15
#define NEWS_MAX_TYPES          10

DECLARE_DO_FUN( do_editnews );

typedef struct news_data NEWS;
struct news_data
{
  NEWS *next;
  NEWS *prev;
  int  number;
  int  type;
  char *title;
  char *name;
  char *post;
  char *date;
};

typedef struct news_type NEWS_TYPE;
struct news_type
{
  NEWS *first_news;
  NEWS *last_news;
  NEWS_TYPE *next;
  NEWS_TYPE *prev;
  int   vnum;
  sh_int level;
  char *header;
  char *cmd_name;
  char *name;
};

NEWS_TYPE *first_news_type;
NEWS_TYPE *last_news_type;

/* news.c */
NEWS *grab_news args((NEWS_TYPE *type, char *str));
NEWS_TYPE *figure_type args((char *str));
void display_news args((CHAR_DATA *ch, NEWS *news, NEWS_TYPE *type));
void renumber_news args((void));
void save_news args((void));
void load_news args((void));
void fread_news args((NEWS *news, FILE *fp));
char *stamp_time         args((void));
void write_html_news  args((void));
void snarf_news args((FILE *fpWrite));
void display_news_type args((CHAR_DATA *ch, NEWS_TYPE *type, char *argument));
void fread_news_type args(( NEWS_TYPE *type, FILE *fp ));
bool news_cmd_hook args((CHAR_DATA *ch, char *cmd, char *argument));
void link_news_to_type args((NEWS *news));
