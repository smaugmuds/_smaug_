/* Adminlist Module
 * Diplays all immortal members who belong to councils
 * Last Modified:  Feb. 24, 1999
 * Mystaric
 */
 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
  #include <dirent.h>
#else
  #define strcasecmp strcmp
#endif
#include "mud.h"

void sort_adminlist( void );

#define ADMINLIST_FILE   SYSTEM_DIR "ADMINLIST"

typedef struct  adminent                  ADMINENT;

struct  adminent
{
    ADMINENT *            next;
    ADMINENT *            last;
    char *              name;
    char *              council;
    sh_int              level;
};

ADMINENT *        first_admin;
ADMINENT *        last_admin;


void toadminfile( const char *line, FILE *wfp )
{
  int filler, xx;
  char outline[MAX_STRING_LENGTH];

  outline[0] = '\0';

  if ( line && line[0] != '\0' )
  {
   filler = ( 78-strlen( line ) );
   if ( filler < 1 )
     filler = 1;
   filler /= 2;
   for ( xx = 0; xx < filler; xx++ )
      strcat( outline, " " );
    strcat( outline, line );
  }
  strcat( outline, "\n\r" );
  fputs( outline, wfp );
}

void add_to_adminlist( char *name, int level )
{
  ADMINENT *admin;

#ifdef DEBUG
  log_string( "Adding to adminlist..." );
#endif

  CREATE( admin, ADMINENT, 1 );
  admin->name = str_dup( name );
  admin->level = level;
  admin->council  = "none";

  if ( !first_admin )
  {
    admin->last = NULL;
    admin->next = NULL;
    first_admin = admin;
    last_admin = admin;
    return;
  }

  admin->last = last_admin;
  admin->next = NULL;
  last_admin->next = admin;
  last_admin = admin;
  return;
}


void remove_tilde( char *str )
{
  for ( ; *str != '\0'; str++ )
        if ( *str == '~' )
          *str = '\0';
  return;
}


void make_adminlist()
{
    DIR *dp;
    struct dirent *dentry;
    char *word;
    FILE *gfp, *wfp;
    int ilevel;
    COUNCIL_DATA *council;
    ADMINENT *admin, *adminnext;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char *arg2 = NULL;

    first_admin = NULL;
    last_admin = NULL;
    

    if ( (dp = opendir( GOD_DIR )) == NULL )
    {
    	bug("Error opening the god directory", 0 );
	return;
    }

    ilevel = 0;
    dentry = readdir( dp );

    while( dentry )
    {
        if ( dentry->d_name[0] != '.' )
        {        
            sprintf (buf, "%s%s", GOD_DIR, dentry->d_name);
            gfp = fopen (buf, "r" );

            if (gfp)
            {
                 word = feof( gfp ) ? "End" : fread_word( gfp );
    	         ilevel = fread_number( gfp );
                 fread_to_eol( gfp );
         	 word = feof( gfp ) ? "End" : fread_word( gfp );
		 add_to_adminlist( dentry->d_name, ilevel );
                 fclose(gfp); 
             }  
        } 
        dentry = readdir( dp );
    } 
    closedir( dp );
    unlink( ADMINLIST_FILE );

    if ( (wfp = fopen( ADMINLIST_FILE, "a" )) == NULL )
    {
    	bug("Error opening the Adminlist file", 0 );
	return;
    }

    sprintf(buf, "%s Administrative Council List", sysdata.mud_name);
    toadminfile(buf, wfp);    
    toadminfile(" ", wfp);
    for ( admin = first_admin; admin; admin = admin->next )
    {
        sprintf (buf, "%s%c/%s", PLAYER_DIR, tolower(admin->name[0]), admin->name);
        gfp = fopen (buf, "r" );
        if (gfp)
        { 
	    arg2 = NULL;
            do
            {    
                fgets( buf, MAX_STRING_LENGTH, gfp);
                arg2 = one_argument(buf, arg);
            }while ( str_cmp(arg,"Council") && !feof(gfp));
        }    
        if (!str_cmp(arg, "Council") )
	{
	     remove_tilde(arg2);
             admin->council = str_dup (arg2);
	}
	fclose( gfp );
    }

    sort_adminlist();
    for (council = first_council; council; council = council->next)
    {
       sprintf( buf, "%s", council->name);
       sprintf( arg, "%c", ' ' );
       toadminfile(buf, wfp);
       for ( admin = first_admin; admin; admin = admin->next )
       {
           if(!str_cmp(admin->council, council->name) )
           {
	      if ( !str_cmp( admin->name, council->head) || ( council->head2 &&
	           !str_cmp( admin->name, council->head2) ) )
              	sprintf(buf, "*%s* ", admin->name);
	      else
              	sprintf(buf, "%s ", admin->name);
              if ((strlen (arg) + strlen (buf)) > 76 )
	      {
                 toadminfile(arg, wfp);
		 sprintf( arg, "%c", ' ');
	      }
              strcat(arg, buf);
           }
        }
   toadminfile (arg, wfp);
   toadminfile (" ", wfp);
   }
   
   for ( admin = first_admin; admin; admin = adminnext )
   {
        adminnext = admin->next;
        DISPOSE(admin->name);
        DISPOSE(admin);
   }
    fclose( wfp );
    return;
} 

void do_makeadminlist( CHAR_DATA *ch, char *argument )
{
  make_adminlist();
}

void sort_adminlist( )
{
  ADMINENT *admin, *tmp, *next;
  COUNCIL_DATA *council;
  bool run = TRUE, head = FALSE, thead = FALSE;

  while ( run )
  {
    run = FALSE;
    for ( admin = first_admin; admin; admin = admin->next )
    {
  	tmp = admin->next;
	if ( tmp )
	{
	   if ( strcmp( admin->name, tmp->name)>0)
	   {
	     if ( tmp->next )
	     	tmp->next->last = admin;
	     if ( admin->last )
	        admin->last->next = tmp;
	     next = tmp->next;
	     tmp->next = admin;
	     tmp->last = admin->last;
	     admin->next = next;
	     admin->last = tmp;
	     if ( last_admin == tmp )
	     	last_admin = admin;
	     if ( first_admin == admin )
	        first_admin = tmp;
	     run = TRUE;
	   }
	}
    }
  }
  run = TRUE;
  while ( run )
  {
    run = FALSE;
    for ( admin = first_admin; admin; admin = admin->next )
    {
  	tmp = admin->next;
	if ( tmp )
	{
	   if ( tmp->level > admin->level )
	   {
	     if ( tmp->next )
	     	tmp->next->last = admin;
	     if ( admin->last )
	        admin->last->next = tmp;
	     next = tmp->next;
	     tmp->next = admin;
	     tmp->last = admin->last;
	     admin->next = next;
	     admin->last = tmp;
	     if ( last_admin == tmp )
	     	last_admin = admin;
	     if ( first_admin == admin )
	        first_admin = tmp;
	     run = TRUE;
	   }
	}
    }
  }
  run = TRUE;
  while ( run )
  {
    run = FALSE;
    for ( admin = first_admin; admin; admin = admin->next )
    {
      head = FALSE;
      thead = FALSE;

      tmp = admin->next;
      if ( tmp )
      {
        for ( council = first_council; council; council = council->next )
        {
      	   if ( !str_cmp( council->head, admin->name) || ( council->head2 &&
	 	!str_cmp( council->head2, admin->name) ) )
		  head = TRUE;
      	   if ( !str_cmp( council->head, tmp->name) || ( council->head2 &&
	 	!str_cmp( council->head2, tmp->name) ) )
		  thead = TRUE;
        }

	   if ( thead && !head )
	   {
	     if ( tmp->next )
	     	tmp->next->last = admin;
	     if ( admin->last )
	        admin->last->next = tmp;
	     next = tmp->next;
	     tmp->next = admin;
	     tmp->last = admin->last;
	     admin->next = next;
	     admin->last = tmp;
	     if ( last_admin == tmp )
	     	last_admin = admin;
	     if ( first_admin == admin )
	        first_admin = tmp;
	     run = TRUE;
	   }
	}
    }
  }
  return;
}

void do_adminlist( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_IMMORT, ch );
    show_file( ch, ADMINLIST_FILE );
}
