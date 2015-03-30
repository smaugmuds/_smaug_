
/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 *------------------------------------------------------------------------  *
 *                 Ident                                                    *
****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>

#ifdef WIN32
  #include <io.h>
  #define EWOULDBLOCK WSAEWOULDBLOCK
#else
  #include <netinet/in.h>
  #include <netdb.h>
  #include <sys/socket.h>
  #define closesocket close
#endif

#include "mud.h"


#ifndef EAGAIN
  #define EAGAIN EWOULDBLOCK
#endif

typedef enum { AS_TOOPEN, AS_TOSEND, AS_TOREAD } AUTH_STATE;

typedef struct auth_data
{
  struct auth_data *next, *prev;
  DESCRIPTOR_DATA *d;
  int afd;
  int state;
  int times;
  struct sockaddr_in us, them;
} AUTH_DATA;

AUTH_DATA *first_auth, *last_auth;

bool auth_read(AUTH_DATA *a, CHAR_DATA *ch);
bool auth_write(AUTH_DATA *a, CHAR_DATA *ch);
bool auth_open(AUTH_DATA *a, CHAR_DATA *ch);

void auth_maxdesc(int *md, fd_set *ins, fd_set *outs, fd_set *excs)
{
  AUTH_DATA *a;
    
  for (a = first_auth; a; a = a->next)
    if (a->state != AS_TOOPEN)
    {
      *md = UMAX(*md, a->afd);
      FD_SET(a->afd, ins);
      FD_SET(a->afd, outs);
      FD_SET(a->afd, excs);
    }
  return;
}

void auth_check(fd_set *ins, fd_set *outs, fd_set *excs)
{
  AUTH_DATA *a, *a_next;
  CHAR_DATA *ch;
  bool ferr;

  for (a = first_auth; a; a = a_next)
  {
    a_next = a->next;
    ferr = FALSE;
    ch = (a->d->original ? a->d->original : a->d->character);
    if (a->state == AS_TOOPEN)
    {
      if (!auth_open(a, ch))
        ferr = TRUE;
    }
    else if (FD_ISSET(a->afd, excs))
    {
      FD_CLR(a->afd, ins);
      FD_CLR(a->afd, outs);
      bug("Auth_check: exception found for %s@%s.",
          (ch ? ch->name : "(unknown)"), a->d->host);
      STRFREE(a->d->user);
      a->d->user = STRALLOC("Exception");
      ferr = TRUE;
    }
    else if (FD_ISSET(a->afd, ins) && a->state == AS_TOREAD)
    {
      if (!auth_read(a, ch))
      {
        FD_CLR(a->afd, outs);
        ferr = TRUE;
      }
    }
    else if (FD_ISSET(a->afd, outs) && a->state == AS_TOSEND)
    {
      if (!auth_write(a, ch))
        ferr = TRUE;
    }
    if (ferr)
    {
      if (a->state != AS_TOOPEN)
        closesocket(a->afd);
      UNLINK(a, first_auth, last_auth, next, prev);
      DISPOSE(a);
    }
  }
  return;
}

char *break_arg(char **s, char end)
{
  char *ret, *ws;
  
  while (isspace(**s))
    ++*s;
  ret = *s;
  while (**s && **s != end)
    ++*s;
  for (ws = (*s)-1; isspace(*ws); --ws)
    *ws = '\0';
  if (**s != '\0')
  {
    **s = '\0';
    ++*s;
  }
/*  bug("break_arg: broke '%s', left '%s'", ret, *s);*/
  return ret;
}

#define KILLRET(bs, us) \
do { \
  bug(bs, (ch ? ch->name : "(unknown)"), a->d->host); \
  STRFREE(a->d->user); \
  a->d->user = STRALLOC(us); \
  return FALSE; \
} while(0)

bool auth_read(AUTH_DATA *a, CHAR_DATA *ch)
{
  char readbuf[MAX_STRING_LENGTH];
  char system[MAX_INPUT_LENGTH], user[MAX_INPUT_LENGTH];
  char *s = readbuf;
  int n;
  
  n = recv(a->afd, readbuf, sizeof(readbuf)-10, 0);
  if (n < 0)
  {
    perror("auth_read: read");
    KILLRET("Auth_read: Error on read for %s@%s.", "(Error on read)");
  }
  if (!n)
    return TRUE;
/*    KILLRET("Auth_read: EOF on read for %s@%s.", "(EOF on read)");*/
  readbuf[n] = '\0';
  while (isspace(*s))
    ++s;
  if (!*s)
    KILLRET("Auth_read: blank auth for %s@%s.", "(blank auth)");
  /* That 1024 check should actually be against the port the user is
     logged into, but in SMAUG theres like 4 possible, and I don't
     feel like checking every single one.. -- Alty */
/*  bug("ident = '%s'.", s);*/
  if (!atoi(break_arg(&s, ',')) || atoi(break_arg(&s, ':')) < 1024)
    KILLRET("Auth_read: Invalid ident reply for %s@%s.", "(invalid ident)");
  break_arg(&s, ':');
  sprintf(system, "%.*s", (int)sizeof(system)-1, break_arg(&s, ':'));
/*  if (!str_cmp(system, "OTHER"))
    KILLRET("Auth_read: invalid system for %s@%s.", "(invalid system)");*/
  sprintf(user, "%.*s", (int)sizeof(user)-1, break_arg(&s, ' '));
/*
  if (!*user)
    KILLRET("Auth_read: no username for %s@%s.", "(no username)"); */
  sprintf(log_buf, "Auth reply ok.  Incoming user [%s@%s] for %s.",
          user, a->d->host, (ch ? ch->name : "(unknown)"));
  STRFREE(a->d->user);
  a->d->user = STRALLOC(user);
  return FALSE; /* FALSE actually removes the AUTH_DATA, which is good. */
}

bool auth_write(AUTH_DATA *a, CHAR_DATA *ch)
{
  char authbuf[32];
  int n;
  
  sprintf(authbuf, "%u , %u\r\n", (unsigned int)ntohs(a->them.sin_port),
          (unsigned int)ntohs(a->us.sin_port));
  n = send(a->afd, authbuf, strlen(authbuf), 0);
  
  if (n != strlen(authbuf))
  {
    if (!--a->times)
    {
      /* This used to be the KILLRET you see below, but too many imms complained
         about log being spammed.. yeah.. like it aint spammed anyways..
         -- Alty */
      STRFREE(a->d->user);
      a->d->user = STRALLOC("(broken pipe)");
      return FALSE;
    }
/*      KILLRET("Auth_write: broken pipe for %s@%s.", "(broken pipe)");*/
    closesocket(a->afd);
    a->state = AS_TOOPEN;
    if (a->times == sysdata.ident_retries-1)
    {
      STRFREE(a->d->user);
      a->d->user = STRALLOC("(pipe breaking)");
    }
    return TRUE; /* Dont kill AUTH_DATA */
  }
  a->state = AS_TOREAD;
  return TRUE;
}

bool auth_open(AUTH_DATA *a, CHAR_DATA *ch)
{
  struct sockaddr_in sock;
  struct servent *serv;
  int err;
#ifdef WIN32
  unsigned int arg = 1;
#endif
  
  a->afd = socket(AF_INET, SOCK_STREAM, 0);
  err = errno;
  if (a->afd < 0)
  {
    perror("auth_open: socket");
    if (err == EAGAIN)
    {
      bug("Auth_open: can't allocate filedesc for %s@%s.",
          (ch ? ch->name : "(unknown)"), a->d->host);
      --a->times;
      return TRUE;
    }
    KILLRET("Auth_open: unknown socket error", "(socket error)");
  }
#ifndef FNDELAY
#define FNDELAY O_NDELAY
#endif
#ifdef WIN32
  if (ioctlsocket(a->afd, FIONBIO, &arg) == -1 )
#else
  if (fcntl(a->afd, F_SETFL, FNDELAY) < 0)
#endif
  {
    perror("auth_open: fcntl");
    KILLRET("Auth_open: unknown fcntl error", "(fcntl error)");
  }
  sock = a->them;
  serv = getservbyname("ident", "tcp");
  if (!serv)
    sock.sin_port = htons(113);
  else
    sock.sin_port = serv->s_port;
  sock.sin_family = AF_INET;
  
  if (connect(a->afd, (struct sockaddr *)&sock, sizeof(sock)) < 0
#ifdef WIN32
  && errno != WSAEINPROGRESS )
#else
  && errno != EINPROGRESS )
#endif
  {
#ifndef WIN32
    if (errno != ECONNREFUSED)
    {
      perror("auth_open: connect");
      KILLRET("Auth_open: unknown connect error", "(connect error)");
    }
#endif
    KILLRET("Auth_open: connection refused", "(connect refused)");
  }
  a->state = AS_TOSEND;
  return TRUE;
}
#undef KILLRET

#define ENDRET(bs, us) \
do { \
  bug(bs, (ch ? ch->name : "(unknown)"), d->host); \
  STRFREE(d->user); \
  d->user = STRALLOC(us); \
  return; \
} while(0)



void set_auth(DESCRIPTOR_DATA *d)
{
  CHAR_DATA *ch = (d->original ? d->original : d->character);
  AUTH_DATA *a;
  struct sockaddr_in us, them;
  int ulen = sizeof(us), tlen = sizeof(them);
  
  /* To stop an uninitialized memory read --Shaddai */
  us.sin_port =    0;
  them.sin_port =  0;

  if (sysdata.ident_retries <= 0)
  {
    STRFREE(d->user);
    d->user = STRALLOC("(ident not active)");
    return;
  }
  if (getsockname(d->descriptor, (struct sockaddr *)&us, &ulen) < 0)
  {
    perror("set_auth: getsockname");
    ENDRET("Set_auth: getsockname error for %s@%s.", "(getsockname error)");
  }
  if (getpeername(d->descriptor, (struct sockaddr *)&them, &tlen) < 0)
  {
    perror("set_auth: getpeername");
    ENDRET("Set_auth: getpeername error for %s@%s.", "(getpeername error)");
  }
  CREATE(a, AUTH_DATA, 1);
  a->d = d;
  a->state = AS_TOOPEN;
  a->times = sysdata.ident_retries;
  a->us = us;
  a->them = them;
  LINK(a, first_auth, last_auth, next, prev);
  STRFREE(d->user);
  d->user = STRALLOC("(in progress)");
  return;
}

void kill_auth(DESCRIPTOR_DATA *d)
{
  AUTH_DATA *a;
  
  for (a = first_auth; a; a = a->next)
    if (a->d == d)
    {
      if (a->state != AS_TOOPEN)
        closesocket(a->afd);
      UNLINK(a, first_auth, last_auth, next, prev);
      DISPOSE(a);
      STRFREE(d->user);
      d->user = STRALLOC("(killed)");
      return;
    }
  return;
}
