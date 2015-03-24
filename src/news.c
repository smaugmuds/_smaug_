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
 * File: news.c
 * Name: Extended News (v2.81)
 * Author: John 'Noplex' Bellone (john.bellone@flipsidesoftware.com)
 * Terms:
 * If this file is to be re-disributed; you must send an email
 * to the author. All headers above the #include calls must be
 * kept intact.
 * Description:
 * This is the extended news module; it allows for news to be
 * posted in note-like format; and bringing you into a editbuffer
 * instead of one-line posts. It also allows support for online
 * HTML output for news to be automatically generated and included
 * via a PHP; SSL; or a TXT include.
 */
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "mud.h"

#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

#ifdef FCLOSE
#undef FCLOSE
#endif
#define FCLOSE(fp) fclose(fp); fp=NULL;

/* locals */
static char local_buf[MAX_INPUT_LENGTH];
int top_news_type;

/* the lovely; and useful; command table */
char * news_command_table[NEWS_MAX_TYPES];

/* olc editnews command */
void do_editnews( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  if(IS_NPC(ch) || !IS_IMMORTAL(ch))
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  set_char_color( AT_GREEN, ch );

  switch( ch->substate )
  {
       default:
         break;

       case SUB_NEWS_POST:
       {
         NEWS *news = NULL;

         news = ch->dest_buf;
         STRFREE(news->post);
         news->post = copy_buffer(ch);
         stop_editing( ch );
         ch->substate = ch->tempnum;
         renumber_news();
         save_news();
         return;
       }
       break;

       case SUB_NEWS_EDIT:
       {
         NEWS *news = NULL;

         news = ch->dest_buf;
         STRFREE(news->post);
         news->post = copy_buffer(ch);
         stop_editing( ch );
         ch->substate = ch->tempnum;
         renumber_news();
         save_news();
         return;
       }
       break;
  }

  argument = one_argument(argument, arg);

  if(arg[0] == '\0' )
  {
    send_to_char( "Syntax: editnews addtype <name>\n\r"
                  "        editnews addnews <type> <subject>\n\r"
                  "        editnews removetype <number>\n\r"
                  "        editnews removenews <type> <number>\n\r"
                  "        editnews edittype <field> <value>\n\r"
                  "        editnews editnews <type> <number> <new subject [optional]>\n\r"
                  " Fields being one of the following:\n\r"
                  " name header cmd_name level\n\r", ch);
    return;
  }

  if( !str_cmp(arg, "save"))
  {
    renumber_news();
    save_news();
    send_to_char( "News saved.\n\r", ch );
    return;
  }

  if(!str_cmp(arg, "addtype"))
  {
    NEWS_TYPE *type = NULL;

    if(argument[0] == '\0')
    {
      send_to_char("Syntax: editnews addtype <name>\n\r", ch);
      return;
    }

    if(top_news_type >= NEWS_MAX_TYPES)
    {
      send_to_char("There are too many news types.\n\r", ch);
      return;
    }

    CREATE(type, NEWS_TYPE, 1);
    type->name              = STRALLOC(argument);
    type->cmd_name          = STRALLOC(argument);
    type->vnum              = top_news_type++;
    type->level		    = -1;

    news_command_table[type->vnum] = STRALLOC(type->cmd_name);

    LINK(type, first_news_type, last_news_type, next, prev);
    ch_printf( ch, "Newstype '%s' created.\n\r", argument );
    return;
  }

  if(!str_cmp(arg, "removetype"))
  {
    NEWS_TYPE *type = NULL;

    if(argument[0] == '\0')
    {
      send_to_char("Syntax: editnews removetype <name>\n\r", ch);
      return;
    }

    if((type = figure_type(argument)) == NULL)
    {
      send_to_char("Invaild newstype.\n\r", ch);
      return;
    }

    UNLINK(type, first_news_type, last_news_type, next, prev);
    if(type->name)
     STRFREE(type->name);
    if(type->header)
     STRFREE(type->header);
    if(type->cmd_name)
     STRFREE(type->cmd_name);
    if(news_command_table[type->vnum])
     STRFREE(news_command_table[type->vnum]);
    if(news_command_table[type->level])
     STRFREE(news_command_table[type->level]);
    {
      NEWS *news = NULL;
      void *next;

      for(news = type->first_news; news; news = next)
      {
        next = news->next;
        UNLINK( news, type->first_news, type->last_news, next, prev );
        if(news->name)
         STRFREE(news->name);
        if(news->title)
         STRFREE(news->title);
        if(news->date)
         STRFREE( news->date );
        if(news->post)
         STRFREE( news->post );
        DISPOSE(news);
      }
    }
    DISPOSE(type);
    top_news_type--;
    renumber_news();
    save_news();
    ch_printf( ch, "Newstype '%s' removed.\n\r", argument );
    return;
  }

  if(!str_cmp(arg, "edittype"))
  {
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    NEWS_TYPE *type = NULL;

    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    if(arg2[0] == '\0' || arg3[0] == '\0')
    {
      send_to_char("Syntax: editnews edittype <type> <field> <value>\n\r", ch);
      send_to_char("Fields being one of the following:\n\r"
                   "name header cmd_name level\n\r", ch );
      return;
    }

    if((type = figure_type(arg2)) == NULL)
    {
      send_to_char("Invalid newstype.\n\r", ch);
      return;
    }

    if(!str_cmp(arg3, "cmd_name"))
    {
      type->cmd_name  = STRALLOC(argument);
      news_command_table[type->vnum] = STRALLOC(type->cmd_name);
      send_to_char("Cmd_name set.\n\r", ch);
      save_news();
      return;
    }
    else if(!str_cmp(arg3, "name"))
    {
      type->name           = STRALLOC(argument);
      send_to_char("Name set.\n\r", ch);
      save_news();
      return;
    }
    else if(!str_cmp(arg3, "level"))
    {
      if ( argument[0] == '\0' )
      {
	ch_printf( ch, "%d\n\r", type->level );
	return;
      }
      else
	type->level = atoi(argument);
      send_to_char("Level set.\n\r", ch);
      save_news();
      return;
    }
    else
    {
      send_to_char("Syntax: editnews edittype <type> <field> <value>\n\r", ch);
      send_to_char("Fields being one of the following:\n\r"
                   "name header cmd_name level\n\r", ch );
      return;
    }
  }

  if( !str_cmp(arg, "addnews" ))
  {
    char arg[MAX_INPUT_LENGTH];
    NEWS_TYPE *type = NULL;
    NEWS *news = NULL;

    argument = one_argument(argument, arg);

    if(arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char( "Syntax: editnews addnews <type> <subject>\n\r", ch );
      return;
    }

    if((type = figure_type(arg)) == NULL)
    {
      send_to_char("Invaild newstype. Use 'newstypes' to get a valid listing.\n\r", ch);
      return;
    }

    CREATE( news, NEWS, 1 );
    news->title   = STRALLOC(argument);
    news->name    = STRALLOC(ch->name);
    news->date    = STRALLOC(stamp_time( ));
    news->post    = STRALLOC("");

    /* pop character into a writing buffer */
    if( ch->substate == SUB_REPEATCMD )
       ch->tempnum = SUB_REPEATCMD;
    else
       ch->tempnum = SUB_NONE;

    ch->substate = SUB_NEWS_POST;
    ch->dest_buf = news;
    start_editing( ch, news->post );
    LINK(news, type->first_news, type->last_news, next, prev);
    return;
  }

  if( !str_cmp(arg, "editnews"))
  {
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    NEWS *news = NULL;
    NEWS_TYPE *type = NULL;

    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    if(arg2[0] == '\0')
    {
      send_to_char( "Syntax: editnews editnews <type> <number> <new subject [optional]>\n\r", ch );
      return;
    }

    /* changed for new -newstype- indexing - 5/5/02 */
    if((type = figure_type(arg2)) == NULL)
    {
      send_to_char("Invalid newstype. Use 'newstypes' to get a valid listing.\n\r", ch);
      return;
    }

    if((news = grab_news(type, arg3)) == NULL)
    {
      pager_printf_color(ch, "That's not a valid news number.\n\rUse '%s' to view the valid numbers.\n\r", type->cmd_name );
      return;
    }

    /* a changed title */
    if( argument[0] != '\0' )
      news->title   = STRALLOC( argument );

    /* new date news was edited */
    news->date      = STRALLOC(stamp_time( ));
    /* pop character into a writing buffer */
    if( ch->substate == SUB_REPEATCMD )
       ch->tempnum = SUB_REPEATCMD;
    else
       ch->tempnum = SUB_NONE;

    ch->substate = SUB_NEWS_EDIT;
    ch->dest_buf = news;
    start_editing( ch, news->post );
    return;
  }

  if( !str_cmp(arg, "removenews"))
  {
    char arg2[MAX_INPUT_LENGTH];
    NEWS *news = NULL;
    NEWS_TYPE *type = NULL;

    argument = one_argument(argument, arg2);
    if(argument[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char("Syntax: editnews remove <number>\n\r", ch );
      return;
    }

    /* changed for new -newstype- indexing - 5/5/02 */
    if((type = figure_type(arg2)) == NULL)
    {
      send_to_char("Invalid newstype. Use 'newstypes' to get a valid listing.\n\r", ch);
      return;
    }

    if((news = grab_news(type, argument)) == NULL)
    {
      send_to_char( "Type 'news' to gain a list of the news numbers.\n\r", ch );
      return;
    }

    UNLINK( news, type->first_news, type->last_news, next, prev );
   if(news->name)
    STRFREE( news->name );
   if(news->title)
    STRFREE( news->title );
   if(news->date)
    STRFREE( news->date );
   if(news->post)
    STRFREE( news->post );
    DISPOSE( news );
    renumber_news();
    save_news();
    send_to_char("News item removed.\n\r", ch );
    return;
  }
}

/* figure the type of a newstype by the vnum
 * or by the cmd_name off the news_cmd_table -Nopey */
NEWS_TYPE *figure_type(char *str)
{
  if(is_number(str))
  {
    NEWS_TYPE *type = NULL;
    int number = atoi(str);

    /* poll the list for the vnum */
    for(type = first_news_type; type; type = type->next)
     if(type->vnum == number)
      return type;
  }
  else /* a cmd name */
  {
    NEWS_TYPE *type = NULL;
    int x;

    /* poll the cmd_name array for word */
    for(x = 0; x < top_news_type; x++)
     if(!str_cmp(str, news_command_table[x]))
     {
       for(type = first_news_type; type; type = type->next)
        if(type->vnum == x)
          return type;
     }
  }
  return NULL;
}

/* Snatch news up from the linked list */
NEWS *grab_news(NEWS_TYPE *type, char *str)
{
  NEWS *news = NULL;

  for( news = type->first_news; news; news = news->next )
  {
    if( news->number == atoi(str))
        return news;
  }
  return NULL;
}

/* display a full news to the character */
/* updated for the new display type 5/1/02 */
void display_news( CHAR_DATA *ch, NEWS *news, NEWS_TYPE *type )
{
  pager_printf_color( ch, "\n\r&g--------------------------------------\n\r" );
  pager_printf_color( ch, type->header);
  pager_printf_color( ch, "&g--------------------------------------\n\r" );
  pager_printf_color( ch, NEWS_HEADER_READ);
  sprintf( local_buf, "&g(&W%2d&g)  &W%-12s &%-11s  &W%s&g\n\r", news->number, news->name, news->date, news->title );
  pager_printf_color( ch, local_buf );
  pager_printf_color( ch, "\n\r" );
  if ( news->post[0] != '\0' )
        pager_printf_color( ch, news->post );
  else
        pager_printf_color( ch, "&gNo further information.\n\r" );
  pager_printf_color( ch, "&g--------------------------------------\n\r" );
  pager_printf_color( ch, "\n\r" );
  return;
}

/* renumber the news */
/* changed for new indexing - 5/5/02 */
void renumber_news(void)
{
  NEWS_TYPE *type = NULL;
  NEWS *news = NULL;
  int x, y;

  for(y = 0; y < top_news_type; y++)
   if(news_command_table[y])
    STRFREE(news_command_table[y]);

  top_news_type = 0;

  for(type = first_news_type; type; type = type->next)
  {
    type->vnum = top_news_type++;
    news_command_table[type->vnum] = STRALLOC(type->cmd_name);

    x = 0;
    for(news = type->first_news; news; news = news->next)
    {
      x++;
      news->number = x;
      news->type = type->vnum;
    }
  }
  return;
}

/* save the linked list */
/* changed for new indexing - 5/5/02 */
void save_news(void)
{
  NEWS *news = NULL;
  NEWS_TYPE *type = NULL;
  FILE *fp = NULL;
  char filename[256];

  sprintf( filename, "%s%s", SYSTEM_DIR, NEWS_FILE );
  if((fp = fopen(filename, "w")) == NULL)
  {
    perror("save_news(): cannot open file");
    return;
  }

  for(type = first_news_type; type; type = type->next)
  {
    fprintf(fp, "#NEWSTYPE\n");
    fprintf(fp, "Name          %s~\n", type->name);
    fprintf(fp, "Cmd_Name      %s~\n", type->cmd_name);
    fprintf(fp, "Header        %s~\n", type->header);
    fprintf(fp, "Vnum          %d\n", type->vnum);
    fprintf(fp, "Level	       %d\n", type->level);
    fprintf(fp, "End\n");
    for(news = type->first_news; news; news = news->next)
    {
      fprintf( fp, "#NEWS\n" );
      fprintf( fp, "Title    %s~\n", news->title );
      fprintf( fp, "Name     %s~\n", news->name );
      fprintf( fp, "Date     %s~\n", news->date );
      fprintf( fp, "Type     %d\n", news->type);
      fprintf( fp, "POST     %s~\n", news->post );
      fprintf( fp, "End\n" );
    }
  }
/*
 if(sysdata.news_html_path && sysdata.news_html_path[0] != '\0' && sysdata.max_html_news > 0)
  write_html_news( );
 */
  FCLOSE(fp);
  return;
}

/* load the linked list from disk */
void load_news(void)
{
  FILE *fp = NULL;
  char filename[256];

  sprintf( filename, "%s%s", SYSTEM_DIR, NEWS_FILE );
  if((fp = fopen(filename, "r")) == NULL)
  {
    perror("load_news(): cannot open file");
    return;
  }

  for( ; ; )
  {
    NEWS_TYPE *type = NULL;
    NEWS *news = NULL;
    char *word;
    char letter;

    letter = fread_letter(fp);

    if(letter == '*')
    {
      fread_to_eol(fp);
      continue;
    }

    if(letter != '#')
    {
      bug("load_news(): # not found");
      break;
    }

    word = fread_word(fp);

    if(!str_cmp(word, "NEWS"))
    {
      CREATE( news, NEWS, 1 );
      news->type = -1;

      fread_news(news, fp);
      link_news_to_type(news);
      continue;
    }
    /* added for new indexing - 5/5/02 */
    else if(!str_cmp(word, "NEWSTYPE"))
    {
      CREATE(type, NEWS_TYPE, 1);

      fread_news_type(type, fp);
      LINK(type, first_news_type, last_news_type, next, prev);
      continue;
    }
    if(!str_cmp(word, "END"))
         break;
    else
    {
      bug("load_news(): unknown section %s", word);
      continue;
    }
  }
  FCLOSE(fp);
  renumber_news();
  return;
}

/* added for new indexing - 5/5/02 - Nopey */
/* adds the news to to the correct newstype */
void link_news_to_type(NEWS *news)
{
  NEWS_TYPE *type = NULL;

  sprintf(local_buf, "%d", news->type);
  if((type = figure_type(local_buf)) == NULL)
  {
    bug("link_news_to_type(): invaild news->type %d", news->type);
    return;
  }
  LINK(news, type->first_news, type->last_news, next, prev);
  return;
}

void fread_news( NEWS *news, FILE *fp )
{
  char *word;
  bool fMatch;

  for( ; ; )
  {
    word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;

   switch(UPPER(word[0]))
   {
    case '*':
     fread_to_eol(fp);
     break;

    case 'D':
     KEY( "Date", news->date, fread_string(fp));
     break;

    case 'E':
     if(!str_cmp(word, "END"))
     {
       if(!news->name)
        news->name = STRALLOC("Unknown");

       if(!news->date)
       {
        news->date = STRALLOC(stamp_time( ));
       }

       if(!news->title)
        news->title = STRALLOC("News Post");

       if(news->type <= -1)
        news->type = 0;
       return;
     }
    break;

    case 'N':
     KEY( "Name", news->name, fread_string(fp));
     break;

    case 'P':
     if(!str_cmp(word, "POST"))
     {
      fMatch = TRUE;
      news->post = fread_string(fp);
      break;
     }
    break;

    case 'T':
     KEY( "Title", news->title, fread_string(fp));
     KEY("Type", news->type, fread_number(fp));
     break;
   }

   if(!fMatch)
     bug("fread_news(): no match: %s", word);
  }
}

/* added for new index - 5/5/02 - Nopey */
void fread_news_type( NEWS_TYPE *type, FILE *fp )
{
  char *word;
  bool fMatch;

  for( ; ; )
  {
    word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;

   switch(UPPER(word[0]))
   {
    case '*':
     fread_to_eol(fp);
     break;

    case 'C':
     KEY( "Cmd_Name", type->cmd_name, fread_string(fp));
    break;

    case 'E':
     if(!str_cmp(word, "END"))
     {
       if(!type->name)
        type->name = STRALLOC("Unknown");

       return;
     }
    break;

    case 'H':
     KEY("Header", type->header, fread_string(fp));
    break;

    case 'L':
     KEY( "Level", type->level, fread_number(fp));
     break;

    case 'N':
     KEY( "Name", type->name, fread_string(fp));
     break;

    case 'V':
     KEY( "Vnum", type->vnum, fread_number(fp));
     break;
   }

   if(!fMatch)
     bug("fread_news_type(): no match: %s", word);
  }
}

/* stamp date in mm/dd/yy. return string */
char *stamp_time(void)
{
  static char buf[128];
  struct tm *time;

  time = localtime(&current_time);
  strftime(buf, sizeof(buf), "%x", time);
  return buf;
}

#ifdef NOHTMLISON
/*
 * html the news up!  -Nopey
 */
void write_html_news(void)
{
  FILE *fp = NULL;
  char filename[256];

  sprintf(filename, "%s%s", sysdata.news_html_path, NEWS_INCLUDE_FILE);
  if((fp = fopen(filename, "w")) == NULL)
  {
    bug("write_html_news(): cannot open %s for writing", filename);
    return;
  }
 snarf_news(fp);
 FCLOSE(fp);
 return;
}

/*
 * rip it apart! =\ -Nopey
 */
void snarf_news(FILE *fp)
{
  NEWS *news = NULL;
  int x = 0;
  char buf[1024];

  for(news = last_news; x < sysdata.max_html_news; news = news->prev)
  {
    x++;
    fprintf(fp, "<div align='center'>");
    fprintf(fp, "\n<table width='399' border='1' height='56' bgcolor='#990000'>" );
    sprintf(buf, "\n<tr><td><font face='Arial, Helvetica, sans-serif' size='2'>%s</font></tr></td><tr><td><font size='1' face='Arial, Helvetica, sans-serif' color='#FFFFFF'>[</font><font color='#FFFFFF' size='2'>%s</font><font size='1' color='#FFFFFF'>]</font>", news->title, news->name );
    fprintf(fp, buf);
    fprintf(fp, "\n<font size='1'><font face='Arial, Helvetica, sans-serif'>[<b><font size='2' color='#FFFFFF'>" );
    sprintf(buf, "\n%s\n\r</font></b>]</font></font></td></tr><tr><td height='2' bgcolor='#000000'>", news->date );
    fprintf(fp, buf);
    sprintf(buf, "\n<p><font face='Arial, Helvetica, sans-serif' size='2' color='#FFFFFF'>%s</font><p></td></tr></table></div>", news->post );
    fprintf(fp, buf);
    fprintf(fp, "</div>");
  }
  /* this must stay here -- line below */
  fprintf(fp, "\n<center><font size='2' face='Arial, Helvetica, sans-serif' color='#FFFFFF'>Extended News v2.5 written by: <a href='mailto:noplex@crimsonblade.org'>Noplex</a>; <a href='http://www.crimsonblade.org/snippets/' target='new'>Get your copy here!</a></font></center>\n\r");
  fprintf(fp, "<pre><center>Page last written: %s</center></pre>", ctime(&current_time));
  return;
}
#endif

/* news command hook; interp.c -Nopey */
bool news_cmd_hook(CHAR_DATA *ch, char *cmd, char *argument)
{
  int x = 0;

  for(x = 0; x < top_news_type; x++)
   if(!str_cmp(cmd, news_command_table[x]))
   {
     NEWS_TYPE *type = NULL;

     sprintf(local_buf, "%d", x);
     if((type = figure_type(local_buf)) == NULL)
     {
       bug("news_cmd_hook(): cannot find type for cmd %s", cmd);
       return FALSE;
     }
     if ( get_trust( ch ) < type->level )
	return FALSE;

     display_news_type(ch, type, argument);
     return TRUE;
   }
  return FALSE;
}

/*
 * display the news entry from the command hook -Nopey
 */
void display_news_type(CHAR_DATA *ch, NEWS_TYPE *type, char *argument)
{
  if(!type->first_news)
  {
    send_to_char_color("&gThere are currently no news items for this news type.\n\r", ch);
    return;
  }

  if(argument[0] == '\0' || !str_cmp(argument, "all"))
  {
    bool all_news = FALSE;
    NEWS *news = NULL;
    int x = type->last_news->number, y = NEWS_VIEW;
    int skipper = (x-y);

    if(!str_cmp(argument, "all"))
     all_news = TRUE;

    pager_printf_color( ch, "\n\r&g--------------------------------------\n\r" );
    if(type->header)
     pager_printf_color(ch, type->header);
    pager_printf_color( ch, "&g--------------------------------------\n\r" );
    pager_printf_color(ch, NEWS_HEADER_ALL);

    for(news = type->first_news; news; news = news->next)
    {
      if(!all_news)
      {
        if(skipper > -1)
        {
          skipper--;
          continue;
        }
      }
      sprintf( local_buf, "&g(&W%2d&g)  &W%-12s &%-11s  &W%s&g\n\r", news->number, news->name, news->date, news->title );
      pager_printf_color(ch, local_buf);
    }

    if(!all_news)
    {
      if ( type->last_news->number == 1 )
	pager_printf_color( ch, "&g\n\rThere is one news item.\n\r" );
      else if ( type->last_news->number >  NEWS_VIEW - 1 )
	pager_printf_color( ch, "\n\r&gThere are &w%d&g total items, the oldest of which are not listed here.\n\rUse '&w%s all&g' to list them all.&g\n\r", type->last_news->number, type->cmd_name );
      else
	pager_printf_color( ch, "\n\r&gThere are &w%d&g total items.\n\r", type->last_news->number );
      pager_printf_color( ch, "\n\r&gTo read individual items type '&w%s <number>&g'.\n\r", type->cmd_name );
      return;
    }
    pager_printf_color( ch, "\n\r&gTo read individual items type '&w%s <number>&g'.\n\r", type->cmd_name );
  }

  {
    NEWS *news = NULL;

    if((news = grab_news(type, argument)) == NULL)
    {
      if( str_cmp( argument, "all" ) )
	send_to_char_color( "&g\n\rThat's not a news post number.\n\rUse '&wnews&g' to view them.\n\r", ch );
      return;
    }
    display_news(ch, news, type);
    return;
  }
}

