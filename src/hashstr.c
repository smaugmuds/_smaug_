/****************************************************************************
 * Advanced string hashing functions (c)1996 D.S.D. Software, written by    *
 * Derek Snider for use in SMAUG.					    *
 *									    *
 * These functions keep track of how many "links" are pointing to the	    *
 * memory allocated, and will free the memory if all the links are removed. *
 * Make absolutely sure you do not mix use of strdup and free with these    *
 * functions, or nasty stuff will happen!				    *
 * Most occurances of strdup/str_dup should be replaced with str_alloc, and *
 * any free/DISPOSE used on the same pointer should be replaced with	    *
 * str_free.  If a function uses strdup for temporary use... it is best if  *
 * it is left as is.  Just don't get usage mixed up between conventions.    *
 * The hashstr_data size is 8 bytes of overhead.  Don't be concerned about  *
 * this as you still save lots of space on duplicate strings.	-Thoric	    *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


#define STR_HASH_SIZE	1024

struct hashstr_data
{
    struct hashstr_data	*next;		/* next hash element */
    unsigned short int	 links;		/* number of links to this string */
    unsigned short int	 length;	/* length of string */
};

char *		str_alloc( char *str );
char *		quick_link( char *str );
int		str_free( char *str );
void		show_hash( int count );
char *		hash_stats( void );

struct hashstr_data *string_hash[STR_HASH_SIZE];

/*
 * Check hash table for existing occurance of string.
 * If found, increase link count, and return pointer,
 * otherwise add new string to hash table, and return pointer.
 */
char *str_alloc( char *str )
{
   register int len, hash, psize;
   register struct hashstr_data *ptr;

   len = strlen(str);
   psize = sizeof(struct hashstr_data);
   hash = len % STR_HASH_SIZE;
   for (ptr = string_hash[hash]; ptr; ptr = ptr->next )
     if ( len == ptr->length && !strcmp(str,(char *)ptr+psize) )
     {
	if ( ptr->links < 65535 )
	  ++ptr->links;
	return (char *) ptr+psize;
     }
   ptr = (struct hashstr_data *) malloc(len+psize+1);
   ptr->links		= 1;
   ptr->length		= len;
   if (len)
     strcpy( (char *) ptr+psize, str );
/*     memcpy( (char *) ptr+psize, str, len+1 ); */
   else
     strcpy( (char *) ptr+psize, "" );
   ptr->next		= string_hash[hash];
   string_hash[hash]	= ptr;
   return (char *) ptr+psize;
}

/*
 * Used to make a quick copy of a string pointer that is known to be already
 * in the hash table.  Function increments the link count and returns the
 * same pointer passed.
 */
char *quick_link( char *str )
{
    register struct hashstr_data *ptr;

    ptr = (struct hashstr_data *) (str - sizeof(struct hashstr_data));
    if ( ptr->links == 0 )
    {
	fprintf(stderr, "quick_link: bad pointer\n" );
	return NULL;
    }
    if ( ptr->links < 65535 )
	++ptr->links;
    return str;
}

/*
 * Used to remove a link to a string in the hash table.
 * If all existing links are removed, the string is removed from the
 * hash table and disposed of.
 * returns how many links are left, or -1 if an error occurred.
 */
int str_free( char *str )
{
    register int len, hash;
    register struct hashstr_data *ptr, *ptr2, *ptr2_next;

    len = strlen(str);
    hash = len % STR_HASH_SIZE;
    ptr = (struct hashstr_data *) (str - sizeof(struct hashstr_data));
    if ( ptr->links == 65535 )				/* permanent */
	return ptr->links;
    if ( ptr->links == 0 )
    {
	fprintf(stderr, "str_free: bad pointer\n" );
	return -1;
    }
    if ( --ptr->links == 0 )
    {
	if ( string_hash[hash] == ptr )
	{
	    string_hash[hash] = ptr->next;
	    free(ptr);
	    return 0;
	}
	for ( ptr2 = string_hash[hash]; ptr2; ptr2 = ptr2_next )
	{
	    ptr2_next = ptr2->next;
	    if ( ptr2_next == ptr )
	    {
		ptr2->next = ptr->next;
		free(ptr);
		return 0;
	    }
	}
	fprintf( stderr, "str_free: pointer not found for string: %s\n", str );
	return -1;
    }
    return ptr->links;
}

void show_hash( int count )
{
    struct hashstr_data *ptr;
    int x, c;

    for ( x = 0; x < count; x++ )
    {
	for ( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ );
	fprintf( stderr, " %d", c );
    }
    fprintf( stderr, "\n" );
}

void hash_dump( int hash )
{
    struct hashstr_data *ptr;
    char *str;
    int c, psize;

    if ( hash > STR_HASH_SIZE || hash < 0 )
    {
	fprintf( stderr, "hash_dump: invalid hash size\n\r" );
	return;
    }
    psize = sizeof(struct hashstr_data);
    for ( c=0, ptr = string_hash[hash]; ptr; ptr = ptr->next, c++ )
    {
	str = (char *) (((int) ptr) + psize);
	fprintf( stderr, "Len:%4d Lnks:%5d Str: %s\n\r",
	  ptr->length, ptr->links, str );
    }
    fprintf( stderr, "Total strings in hash %d: %d\n\r", hash, c );
}

char *check_hash( char *str )
{
   static char buf[1024];
   int len, hash, psize, p = 0, c;
   struct hashstr_data *ptr, *fnd;

   buf[0] = '\0';
   len = strlen(str);
   psize = sizeof(struct hashstr_data);
   hash = len % STR_HASH_SIZE;
   for (fnd = NULL, ptr = string_hash[hash], c = 0; ptr; ptr = ptr->next, c++ )
     if ( len == ptr->length && !strcmp(str,(char *)ptr+psize) )
     {
	fnd = ptr;
	p = c+1;
     }
   if ( fnd )
     sprintf( buf, "Hash info on string: %s\n\rLinks: %d  Position: %d/%d  Hash: %d  Length: %d\n\r",
	  str, fnd->links, p, c, hash, fnd->length );
   else
     sprintf( buf, "%s not found.\n\r", str );
   return buf;
}

char *hash_stats( void )
{
    static char buf[1024];
    struct hashstr_data *ptr;
    int x, c, total, totlinks, unique, bytesused, wouldhave, hilink;

    totlinks = unique = total = bytesused = wouldhave = hilink = 0;
    for ( x = 0; x < STR_HASH_SIZE; x++ )
    {
	for ( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ )
	{
	   total++;
	   if ( ptr->links == 1 )
	     unique++;
	   if ( ptr->links > hilink )
	     hilink = ptr->links;
	   totlinks += ptr->links;
	   bytesused += (ptr->length + 1 + sizeof(struct hashstr_data));
	   wouldhave += (ptr->links * (ptr->length + 1));
	}
    }
    sprintf( buf, "Hash strings allocated:%8d  Total links  : %d\n\rString bytes allocated:%8d  Bytes saved  : %d\n\rUnique (wasted) links :%8d  Hi-Link count: %d\n\r",
	total, totlinks, bytesused, wouldhave - bytesused, unique, hilink );
    return buf;
}

void show_high_hash( int top )
{
    struct hashstr_data *ptr;
    int x, psize;
    char *str;

    psize = sizeof(struct hashstr_data);
    for ( x = 0; x < STR_HASH_SIZE; x++ )
	for ( ptr = string_hash[x]; ptr; ptr = ptr->next )
	  if ( ptr->links >= top )
	  {
	     str = (char *) (((int) ptr) + psize);
 	     fprintf( stderr, "Links: %5d  String: >%s<\n\r", ptr->links, str );
	  }
}
