#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

struct at_color_type at_color_table[AT_MAXCOLOR];

void load_colors( void )
{
  extern char strArea[MAX_INPUT_LENGTH];
  extern FILE *fpArea;
  char buf[20];
  int at;
  
  for (at = 0; at < AT_MAXCOLOR; ++at)
  {
    sprintf(buf, "%d", at);
    at_color_table[at].name = str_dup(buf);
    at_color_table[at].def_color = AT_GREY;
  }
  strcpy(strArea, COLOR_FILE);
  if (!(fpArea = fopen(COLOR_FILE, "r")))
  {
    bug("Load_colors: can't open color file -- bland city time!");
    return;
  }
  for (;;)
  {
    at = (feof(fpArea) ? -1 : fread_number(fpArea));
    if (at < 0)
      break;
    if (at < AT_COLORBASE || at >= AT_TOPCOLOR)
    {
      bug("Load_colors: color %d invalid.", at);
      fread_to_eol(fpArea);
      continue;
    }
    at -= AT_COLORBASE;
    DISPOSE(at_color_table[at].name);
    at_color_table[at].name = fread_string_nohash(fpArea);
    at_color_table[at].def_color = fread_number(fpArea);
  }
  fclose(fpArea);
  fpArea = NULL;
  return;
}

void save_colors( void )
{
  FILE *fp;
  int at;
  
  fclose(fpReserve);
  if (!(fp = fopen(COLOR_FILE, "w")))
  {
    bug("Save_colors: can't open color file.");
    fpReserve = fopen(NULL_FILE, "r");
    return;
  }
  for (at = 0; at < AT_MAXCOLOR; ++at)
    fprintf(fp, "%d %s~ %d\n", at+AT_COLORBASE, at_color_table[at].name,
    		at_color_table[at].def_color);
  fprintf(fp, "-1\n");
  fclose(fp);
  fpReserve = fopen(NULL_FILE, "r");
  return;
}

void do_colorize( CHAR_DATA *ch, char *argument )
{
#define MAX_COLOR 16
  static const char colors[MAX_COLOR] = "xrgObpcwzRGYBPCW";
  static const char *color_names[MAX_COLOR] =
    { "black", "blood", "dgreen", "orange", "dblue", "purple", "cyan",
      "grey", "dgrey", "red", "green", "yellow", "blue", "pink", "lblue",
      "white" };
  char arg[MAX_INPUT_LENGTH], *a;
  int color;
  int at;
  bool blink = FALSE;
  CHAR_DATA *victim;
  bool def = FALSE;
  int level = get_trust(ch);
  
  argument = one_argument(argument, arg);
  if (level >= LEVEL_INFINITE &&
      (!str_cmp(arg, "def") || !str_cmp(arg, "default")))
  {
    def = TRUE;
    argument = one_argument(argument, arg);
    victim = NULL;
  }
  else if (level >= LEVEL_SUB_IMPLEM &&
           (victim = get_char_room(ch, arg)) != NULL)
  {
    if (level < get_trust(victim))
    {
      send_to_char("They might not like that.\n\r", ch);
      return;
    }
    argument = one_argument(argument, arg);
  }
  else
    victim = ch;
  if (!def && IS_NPC(victim))
  {
    send_to_char("Not on NPCs.\n\r", ch);
    return;
  }
  if (!*arg)
  {
    for (at = 0; at < AT_MAXCOLOR; ++at)
    {
      if (def || victim->pcdata->colorize[at] == -1)
        color = at_color_table[at].def_color;
      else
        color = victim->pcdata->colorize[at];
      if (color >= AT_BLINK)
      {
        blink = TRUE;
        color -= AT_BLINK;
      }
      else
        blink = FALSE;
      pager_printf_color(ch, "%-8s: &%c%s%-15s&w^x",
      				at_color_table[at].name,
      				colors[color], (blink ? "+^z" : " "),
      				color_names[color]);
      if (at%3 == 2)
        send_to_pager("\n\r", ch);
    }
    if (at%3 != 0)
      send_to_pager("\n\r", ch);
    send_to_pager("\n\rValid colors are:\n\r", ch);
    for (color = 0; color < MAX_COLOR; ++color)
    {
      if (color)
        set_pager_color(color, ch);
      else
        set_pager_color(AT_PLAIN, ch);
      pager_printf(ch, "%-9s", color_names[color]);
      if (color%8 == 7)
        send_to_pager("\n\r", ch);
    }
    if (color%8 != 0)
      send_to_pager("\n\r", ch);
    set_pager_color(AT_GREY + AT_BLINK, ch);
    send_to_pager("\n\rBlinking ", ch);
    set_pager_color(AT_GREY, ch);
    send_to_pager("can be specified by prefixing the color name with a +."
    			"\n\r", ch);
    set_pager_color(AT_PLAIN, ch);
    return;
  }
  for (at = 0; at < AT_MAXCOLOR; ++at)
    if (!str_prefix(arg, at_color_table[at].name))
      break;
  if (at == AT_MAXCOLOR)
  {
    send_to_char("Not a settable color.\n\r", ch);
    return;
  }
  argument = one_argument(argument, arg);
  if (!*arg)
  {
    send_to_char("Set it to what color?\n\r", ch);
    return;
  }
  if ((level >= LEVEL_INFINITE || !str_cmp(ch->name, "altrag")) && !str_cmp(arg, "name"))
  {
    argument = one_argument(argument, arg);
    if (!*arg)
    {
      send_to_char("Name it what?\n\r", ch);
      return;
    }
    DISPOSE(at_color_table[at].name);
    at_color_table[at].name = str_dup(arg);
    send_to_char("Color name set.\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "def") || !str_cmp(arg, "default"))
  {
    if (def)
      send_to_char("That *IS* the default!\n\r", ch);
    else
    {
      victim->pcdata->colorize[at] = -1;
      send_to_char("Color set to default.\n\r", ch);
    }
    return;
  }
  if (*arg == '+')
  {
    a = arg+1;
    if (!*a)
    {
      if (def)
      {
        if (at_color_table[at].def_color >= AT_BLINK)
        {
          at_color_table[at].def_color -= AT_BLINK;
          send_to_char("Default color will no longer blink.\n\r", ch);
        }
        else
        {
          at_color_table[at].def_color += AT_BLINK;
          send_to_char("Default color will now blink.\n\r", ch);
        }
        save_colors();
      }
      else
      {
        if (victim->pcdata->colorize[at] == -1)
          victim->pcdata->colorize[at] = at_color_table[at].def_color;
        if (victim->pcdata->colorize[at] >= AT_BLINK)
        {
          victim->pcdata->colorize[at] -= AT_BLINK;
          send_to_char("Color will no longer blink.\n\r", ch);
        }
        else
        {
          victim->pcdata->colorize[at] += AT_BLINK;
          send_to_char("Color will now blink.\n\r", ch);
        }
      }
      return;
    }
    blink = TRUE;
  }
  else
    a = arg;
  for (color = 0; color < MAX_COLOR; ++color)
    if (!str_prefix(a, color_names[color]))
      break;
  if (color == MAX_COLOR)
  {
    send_to_char("Invalid color name.\n\r", ch);
    return;
  }
  if (blink)
    color += AT_BLINK;
  if (def)
  {
    at_color_table[at].def_color = color;
    send_to_char("Default color set.\n\r", ch);
    save_colors();
  }
  else
  {
    victim->pcdata->colorize[at] = color;
    send_to_char("Color set.\n\r", ch);
  }
  return;
}
