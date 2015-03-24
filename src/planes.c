#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

PLANE_DATA *first_plane, *last_plane;

void do_plist(CHAR_DATA *ch, char *argument)
{
  PLANE_DATA *p;
  
  send_to_char("Planes:\n\r-------\n\r", ch);
  for (p = first_plane; p; p = p->next)
    ch_printf(ch, "%s\n\r", p->name);
  return;
}

void do_pstat(CHAR_DATA *ch, char *argument)
{
  PLANE_DATA *p;
  char arg[MAX_INPUT_LENGTH];
  
  argument = one_argument(argument, arg);
  if (!(p = plane_lookup(arg)))
  {
    send_to_char("Stat which plane?\n\r", ch);
    return;
  }
  ch_printf(ch, "Name: %s\n\r", p->name);
  return;
}

void do_pset(CHAR_DATA *ch, char *argument)
{
  PLANE_DATA *p;
  char arg[MAX_INPUT_LENGTH];
  char mod[MAX_INPUT_LENGTH];
  
  argument = one_argument(argument, arg);
  if (!*arg)
  {
    send_to_char("Syntax: pset <plane> create\n\r", ch);
    send_to_char("        pset save\n\r", ch);
    send_to_char("        pset <plane> delete\n\r", ch);
    send_to_char("        pset <plane> <field> <value>\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("  Where <field> is one of:\n\r", ch);
    send_to_char("    name\n\r", ch);
    return;
  }
  if (!str_cmp(arg, "save"))
  {
    save_planes();
    send_to_char("Planes saved.\n\r", ch);
    return;
  }
  
  argument = one_argument(argument, mod);
  p = plane_lookup(arg);
  
  if (!str_prefix(mod, "create"))
  {
    if (p)
    {
      send_to_char("Plane already exists.\n\r", ch);
      return;
    }
    CREATE(p, PLANE_DATA, 1);
    p->name = STRALLOC(arg);
    LINK(p, first_plane, last_plane, next, prev);
    send_to_char("Plane created.\n\r", ch);
    return;
  }
  if (!p)
  {
    send_to_char("Plane doesn't exist.\n\r", ch);
    return;
  }
  if (!str_prefix(mod, "delete"))
  {
    UNLINK(p, first_plane, last_plane, next, prev);
    STRFREE(p->name);
    DISPOSE(p);
    check_planes(p);
    send_to_char("Plane deleted.\n\r", ch);
    return;
  }
  if (!str_prefix(mod, "name"))
  {
    if (plane_lookup(argument))
    {
      send_to_char("Another plane has that name.\n\r", ch);
      return;
    }
    STRFREE(p->name);
    p->name = STRALLOC(argument);
    send_to_char("Name changed.\n\r", ch);
    return;
  }
  do_pset(ch, "");
  return;
}

PLANE_DATA *plane_lookup(const char *name)
{
  PLANE_DATA *p;
  
  for (p = first_plane; p; p = p->next)
    if (!str_cmp(name, p->name))
      return p;
  for (p = first_plane; p; p = p->next)
    if (!str_prefix(name, p->name))
      return p;
  return NULL;
}

void save_planes(void)
{
  FILE *fp;
  PLANE_DATA *p;
  
  fclose(fpReserve);
  
  if (!(fp = fopen(PLANE_FILE, "w")))
  {
    perror(PLANE_FILE);
    bug("save_planes: can't open plane file");
    fpReserve = fopen(NULL_FILE, "r");
    return;
  }
  for (p = first_plane; p; p = p->next)
  {
    fprintf(fp, "#PLANE\n");
    fprintf(fp, "Name      %s\n", p->name);
    fprintf(fp, "End\n\n");
  }
  fprintf(fp, "#END\n");
  fclose(fp);
  fpReserve = fopen(NULL_FILE, "r");
  return;
}

#ifdef KEY
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void read_plane(FILE *fp)
{
  PLANE_DATA *p;
  char *word;
  bool fMatch;
  
  CREATE(p, PLANE_DATA, 1);
  for (;;)
  {
    word = (feof(fp) ? "End" : fread_word(fp));
    fMatch = FALSE;
    
    switch(UPPER(*word))
    {
    case 'E':
      if (!str_cmp(word, "End"))
      {
        if (plane_lookup(p->name))
        {
          bug("read_plane: duplicate plane name!");
          STRFREE(p->name);
          DISPOSE(p);
        }
        else
          LINK(p, first_plane, last_plane, next, prev);
        return;
      }
      break;
    case 'N':
      KEY("Name", p->name, fread_string(fp));
      break;
    }
    if (!fMatch)
    {
      bug("read_plane: unknown field '%s'", word);
      fread_to_eol(fp);
    }
  }
  return;
}

void load_planes(void)
{
  extern FILE *fpArea;
  extern char strArea[];
  char *word;
  
  if (!(fpArea = fopen(PLANE_FILE, "r")))
  {
    perror(PLANE_FILE);
    bug("load_planes: can't open plane file for read.");
    return;
  }
  strcpy(strArea, PLANE_FILE);
  
  for (;!feof(fpArea);)
  {
    if (fread_letter(fpArea) != '#')
    {
      bug("load_planes: # not found.");
      break;
    }
    word = fread_word(fpArea);
    if (!str_cmp(word, "END"))
      break;
    else if (!str_cmp(word, "PLANE"))		read_plane(fpArea);
    else
    {
      bug("load_planes: invalid section '%s'.", word);
      break;
    }
  }
  fclose(fpArea);
  fpArea = NULL;
  strcpy(strArea, "$");
  return;
}

void build_prime_plane(void)
{
  PLANE_DATA *p;
  
  CREATE(p, PLANE_DATA, 1);
  memset(p, 0, sizeof(*p));
  p->name = STRALLOC("Prime Material");
  LINK(p, first_plane, last_plane, next, prev);
  return;
}

void check_planes(PLANE_DATA *p)
{
  extern ROOM_INDEX_DATA *room_index_hash[];
  int vnum;
  ROOM_INDEX_DATA *r;
  
  if (!first_plane)
    build_prime_plane();
  
  for (vnum = 0; vnum < MAX_KEY_HASH; ++vnum)
    for (r = room_index_hash[vnum]; r; r = r->next)
      if (!r->plane || r->plane == p)
        r->plane = first_plane;
  return;
}
