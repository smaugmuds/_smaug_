/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc.h: the core protocol definitions
 *
 * Copyright (C) 1996,1997 Oliver Jowett <oliver@jowett.manawatu.planet.co.nz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef IMC_H
#define IMC_H

#include <time.h>
#include <sys/time.h>
#include "imc-config.h"

#define REVISION "Revision 7 4/29/99"
/* activation states */

#define IA_NONE        0
#define IA_CONFIG1     1
#define IA_CONFIG2     2
#define IA_UP          3
#define IA_LISTENING   4

/* connection states */

#define IMC_CLOSED     0
#define IMC_CONNECTING 1
#define IMC_WAIT1      2
#define IMC_WAIT2      3
#define IMC_CONNECTED  4

/* connection flags */

#define IMC_NOAUTO 1
#define IMC_CLIENT 2
#define IMC_RECONNECT 4
#define IMC_BROADCAST 8
#define IMC_DENY 16
#define IMC_QUIET 32
#define IMC_HUB 64      /* SPAM fix - shogar */
#define IMC_MAIN_HUB 128 /* Scion */
#define IMC_OLD_HUB 256 /* Scion */
#define IMC_DEAD_HUB 512 /* Scion */
#define IMC_NEW_HUB 1024 /* shogar */
#define IMC_NOSWITCH 2048 /* shogar */


/* ignore types */
#define IMC_IGNORE  1          /* ignore sender */
#define IMC_NOTRUST 2          /* don't trust sender's levels */
#define IMC_TRUST   3          /* do trust sender's levels */

/* ignore match flags */
#define IMC_IGNORE_EXACT  0    /* match exactly */
#define IMC_IGNORE_PREFIX 1    /* ignore prefixes when matching */
#define IMC_IGNORE_SUFFIX 2    /* ignore suffixes when matching */

/* imc_char_data invisibility state */
#define IMC_INVIS 1
#define IMC_HIDDEN 2




/* typedefs */

/* flag/state tables */
typedef struct
{
  char *name;			/* flag name */
  int value;			/* bit value */
} imc_flag_type;

/* data in an IMC packet */
typedef struct
{
  char *key[IMC_MAX_KEYS];
  char *value[IMC_MAX_KEYS];
} imc_data;

/* an IMC packet, as seen by the high-level code */
typedef struct
{
  char to[IMC_NAME_LENGTH];	/* destination of packet */
  char from[IMC_NAME_LENGTH];	/* source of packet      */
  char type[IMC_TYPE_LENGTH];	/* type of packet        */
  imc_data data;		/* data of packet        */

  /* internal things which only the low-level code needs to know about */
  struct {
    char to[IMC_NAME_LENGTH];
    char from[IMC_NAME_LENGTH];
    char path[IMC_PATH_LENGTH];
    unsigned long sequence;
    int stamp;
  } i;
} imc_packet;

/* an actual IMC connection */
typedef struct _imc_connect
{
  struct _imc_info *info;       /* imc_info struct for this connection */

  int desc;			/* descriptor */
  unsigned short state;	      	/* IMC_xxxx state */
  unsigned short version;      	/* version of remote site */

  short newoutput;              /* try to write at end of cycle regardless
				 * of fd_set state?
				 */
  
  char *inbuf;		        /* input buffer */
  int insize;

  char *outbuf;		        /* output buffer */
  int outsize;

  int spamcounter1;             /* packet counters since last update */
  int spamcounter2;
  int spamtime1;                /* +ve = time exceeding limit */
  int spamtime2;                /* -ve = time remaining with enforced limit */

  struct _imc_connect *next;
} imc_connect;

/* a configured IMC connection */
typedef struct _imc_info
{
  char *name;			/* name of remote mud */
  char *host;			/* hostname */

  struct _imc_connect *connection;

  unsigned short port;		/* remote port */
  char *serverpw;		/* server password */
  char *clientpw;		/* client password */
  int flags;			/* connection flags */
  time_t timer_duration;        /* delay after next reconnect failure */

  int rcvstamp;                 /* packets get this stamp on arrival */
  int noforward;                /* packets with these bits set don't get
                                 * forwarded here */

  time_t last_connected;        /* last connected when? */
  int connect_attempts;       /* try for 3 times - shogar */
  
  struct _imc_info *next;
} imc_info;

/* IMC statistics */
typedef struct
{
  time_t start;		        /* when statistics started */
  
  long rx_pkts;		        /* received packets */
  long tx_pkts;		        /* transmitted packets */
  long rx_bytes;		/* received bytes */
  long tx_bytes;		/* transmitted bytes */

  int max_pkt;                  /* max. size packet processed */
  int sequence_drops;           /* dropped packets due to age */
} imc_statistics;

#define IMC_REMINFO_NORMAL 0
#define IMC_REMINFO_EXPIRED 1

/* info about another mud on IMC */
typedef struct _imc_reminfo
{
  char *name;
  char *version;
  time_t alive;
  int ping;
  int type;
  int hide;
  char *route;
  char *path;
  unsigned long top_sequence;
  struct _imc_reminfo *next;
} imc_reminfo;

/* an event */
typedef struct _imc_event
{
  time_t when;
  void (*callback)(void *data);
  void *data;
  int timed;
  struct _imc_event *next;
} imc_event;

/* for the versioning table */
typedef struct
{
  int version;
  const char *(*generate) (const imc_packet *);
  imc_packet *(*interpret) (const char *);
} _imc_vinfo;

/* an entry in the memory table */
typedef struct
{
  char *from;
  unsigned long sequence;
} _imc_memory;

/* an ignore/notrust/etc entry */
typedef struct _imc_ignore_data
{
  char *name;
  int match;                      /* exact, prefix, suffix */
  int type;                       /* ignore, notrust */
  struct _imc_ignore_data *next;
} imc_ignore_data;

/* a player on IMC */
typedef struct
{
  char name[IMC_NAME_LENGTH];	/* name of character */
  int invis;			/* invisible to IMC? */
  int level;			/* trust level */
  int wizi;			/* wizi level */
} imc_char_data;

/* one piece of mail */
typedef struct _imc_mail
{
  char *from;			/* 'from' line */
  char *to;			/* 'to' line */
  char *text;			/* text of the mail */
  char *date;			/* 'date' line */
  char *subject;		/* 'subject' line */
  char *id;			/* mail ID (should be unique among mails) */
  time_t received;		/* when it was received (into the queue) */
  int usage;			/* number of references to this mail */
  struct _imc_mail *next;
} imc_mail;

/* a queue entry pointing at a piece of mail */
typedef struct _imc_qnode
{
  imc_mail *data;
  char *tomud;
  struct _imc_qnode *next;
} imc_qnode;

/* a mail ID that has been received */
typedef struct _imc_mailid
{
  char *id;			/* message-id */
  time_t received;		/* when received */
  struct _imc_mailid *next;
} imc_mailid;

/* site information */
typedef struct
{
  char *name;    /* FULL name of mud */
  char *host;    /* host AND port */
  char *email;   /* contact address (email) */
  char *imail;   /* contact address (IMC mail) */
  char *www;     /* homepage */
  char *details; /* BRIEF description of mud */
  char *flags;   /* special flags - currently only "hide" is supported */
  int maxguests; /* max number of guest connections - shogar */
} imc_siteinfo_struct;

/* data structures */

extern imc_siteinfo_struct imc_siteinfo;

/* the packet memory table */
extern _imc_memory imc_memory[IMC_MEMORY];
/* the version lookup table */
extern _imc_vinfo imc_vinfo[];

/* global stats struct */
extern imc_statistics imc_stats;

/* flag and state tables */
extern const imc_flag_type imc_connection_flags[];
extern const imc_flag_type imc_ignore_types[];
extern const imc_flag_type imc_state_names[];
extern const imc_flag_type imc_active_names[];

/* global recursion lock */
extern int imc_lock;
/* the local IMC name */
extern char *imc_name;
/* the local IMC port */
extern unsigned short imc_port;
/* IP to bind to */
extern unsigned long imc_bind;
/* the configuration prefix */
extern char *imc_prefix;
/* run as a router? */
extern int imc_is_router;

/* the ignore list */
extern imc_ignore_data *imc_ignore_list;
/* the reminfo list */
extern imc_reminfo *imc_reminfo_list;
/* the connection list */
extern imc_connect *imc_connect_list;
/* the configured connection list */
extern imc_info *imc_info_list;
/* the event list, and recycle list */
extern imc_event *imc_event_list, *imc_event_free;

/* the current time */
extern time_t imc_now;
/* next sequence number to use */
extern unsigned long imc_sequencenumber;

/* IMC state */
extern int imc_active;

/* -1 if prefix is in use (when network is up), >=0 otherwise */
extern int imc_lock_file;

/* mail list */
extern imc_mail *imc_ml_head;
/* mail queue */
extern imc_qnode *imc_mq_head, *imc_mq_tail;
/* ID list */
extern imc_mailid *imc_idlist;

/* last reported error */
extern char imc_lasterror[IMC_DATA_LENGTH];

/* packet hook */
int (*imc_recv_hook)(const imc_packet *p, int bcast);

/* imc-util.c exported functions */

/* static buffer handling */
char *imc_getsbuf(int len);
void imc_shrinksbuf(char *buf);

/* imc data handlers */
const char *imc_getkey(const imc_data *p, const char *key, const char *def);
int imc_getkeyi(const imc_data *p, const char *key, int def);
void imc_addkey(imc_data *p, const char *key, const char *value);
void imc_addkeyi(imc_data *p, const char *key, int value);
void imc_initdata(imc_data *p);
void imc_freedata(imc_data *p);
void imc_clonedata(const imc_data *p, imc_data *n);

/* reminfo handling */
imc_reminfo *imc_find_reminfo(const char *name, int type);
imc_reminfo *imc_new_reminfo(void);
void imc_delete_reminfo(imc_reminfo *p);

/* info handling */
imc_info *imc_new_info(void);
void imc_delete_info(imc_info *);
imc_info *imc_getinfo(const char *name);

/* state/flag handling */
const char *imc_flagname(int value, const imc_flag_type *table);
int imc_flagvalue(const char *name, const imc_flag_type *table);
const char *imc_statename(int value, const imc_flag_type *table);
int imc_statevalue(const char *name, const imc_flag_type *table);

/* string manipulations */
const char *imc_nameof(const char *name);
const char *imc_mudof(const char *name);
const char *imc_makename(const char *name, const char *mud);
const char *imc_firstinpath(const char *path);
const char *imc_lastinpath(const char *path);
const char *imc_getarg(const char *arg, char *buf, int length);
int imc_hasname(const char *list, const char *name);
void imc_addname(char **list, const char *name);
void imc_removename(char **list, const char *name);
void imc_slower(char *what);
void imc_sncpy(char *dest, const char *src, int count);
const char *imc_getconnectname(const imc_connect *c);

/* logging */

void imc_logstring(const char *format,...); /*__attribute__((format(printf,1,2))); Uncomment these */
void imc_logerror(const char *format,...); /*__attribute__((format(printf,1,2))); if your system can*/
void imc_qerror(const char *format,...); /*__attribute__((format(printf,1,2)));   use them. */
void imc_lerror(const char *format,...); /*__attribute__((format(printf,1,2)));*/
const char *imc_error(void);

/* external log interfaces */
void imc_log(const char *string);
void imc_debug(const imc_connect *, int out, const char *packet);



/* imc-events.c exported functions */

/* event handling */
imc_event *imc_new_event(void);
void imc_free_event(imc_event *p);
void imc_add_event(int when, void (*callback)(void *), void *data, int timed);
void imc_cancel_event(void (*callback)(void *), void *data);
void imc_run_events(time_t);
int imc_next_event(void (*callback)(void *), void *data);
void imc_recv_keepalive(const char *from, const char *version, const char *flags); /* shogar */
void imc_request_keepalive(void); /* shogar */
void imc_recv_info_reply(const char *from, const char *hub, int direct); /* Scion */
void imc_send_autoconnect(const char *to); /* Scion */
void imc_recv_autoconnect(const char *to, const char *rhost, int rport, const char *rclientpw, const char *rserverpw, 
						  int rrcvstamp, int rnoforward, const char *rflags, const char *remotename); /* Scion */
void imc_autoconnect_reply_accept(const char *from, const char *rhost, int rport, const char *rclientpw, const char *rserverpw, 
						  int rrcvstamp, int rnoforward, const char *rflags, const char *remotename, const char *confirm); /* Scion */
imc_info *imc_insert_info(); /* shogar */
void imc_cancel_info(imc_info *i); /* shogar */


/* all events (in imc-events.c unless otherwise specified) */
void ev_expire_reminfo(void *data);
void ev_drop_reminfo(void *data);
void ev_login_timeout(void *data);       /* imc.c */
void ev_reconnect(void *data);
void ev_shrink_input(void *data);
void ev_shrink_output(void *data);
void ev_keepalive(void *data);
void ev_request_keepalive(void *data);
void ev_spam1(void *data);
void ev_spam2(void *data);
void ev_qnode_expire(void *data);        /* imc-mail.c */
void ev_mailid_expire(void *data);       /* imc-mail.c */
void ev_qnode_send(void *data);          /* imc-mail.c */
void ev_imc_optimize(void *data);	/* Scion */
void ev_imc_pollforhub(void *data);	/* shogar */

/* imc.c exported functions */

void imc_setup_reconnect(imc_info *i);

const char *imc_make_password(void); /* Scion */

void imc_send(imc_packet *p);
void imc_recv(const imc_packet *p);

void imc_startup_port(void);
void imc_startup_network(void);
void imc_startup(const char *prefix);
void imc_shutdown(void);
void imc_shutdown_network(void);
void imc_shutdown_port(void);
void imc_idle(int s, int us);

int imc_get_max_timeout(void);
int imc_fill_fdsets(int maxfd, fd_set *read, fd_set *write, fd_set *exc);
void imc_idle_select(fd_set *read, fd_set *write, fd_set *exc, time_t now);

int imc_connect_to(const char *mud);
int imc_disconnect(const char *mud);

/* memory allocation hooks
 * these aren't #defines so we don't have to know about the actual mud
 * function declarations in imc.c/imc-comm.c
 */
#define imc_free(b,s) {IMC_free(b,s);b=NULL;}
#define imc_strfree(s) {IMC_strfree(s);s=NULL;}
void *imc_malloc(int size);
void IMC_free(void *block, int size);
char *imc_strdup(const char *src);
void IMC_strfree(char *str);



/* imc-interp.c exported functions */

void imc_sendignore(const char *to);

void imc_send_chat(const imc_char_data *from, int channel,
		   const char *argument, const char *to);
void imc_send_emote(const imc_char_data *from, int channel,
		    const char *argument, const char *to);
void imc_send_tell(const imc_char_data *from, const char *to,
		   const char *argument, int isreply);
void imc_send_who(const imc_char_data *from, const char *to,
		  const char *type);
void imc_send_whoreply(const char *to, const char *data, int sequence);
void imc_send_whois(const imc_char_data *from, const char *to);
void imc_send_whoisreply(const char *to, const char *data);
void imc_send_beep(const imc_char_data *from, const char *to);
void imc_send_keepalive(void);
void imc_send_ping(const char *to, int time_s, int time_u);
void imc_send_pingreply(const char *to, int time_s, int time_u, const char *path);
void imc_send_whois(const imc_char_data *from, const char *to);
void imc_send_whoisreply(const char *to, const char *text);
void imc_send_info_request(void);


void imc_whoreply_start(const char *to);
void imc_whoreply_add(const char *text);
void imc_whoreply_end(void);

/* callbacks that need to be provided by the interface layer */

void imc_recv_chat(const imc_char_data *from, int channel,
		   const char *argument);
void imc_recv_emote(const imc_char_data *from, int channel,
		    const char *argument);
void imc_recv_tell(const imc_char_data *from, const char *to,
		   const char *argument, int isreply);
void imc_recv_whoreply(const char *to, const char *data, int sequence);
void imc_recv_who(const imc_char_data *from, const char *type);
void imc_recv_beep(const imc_char_data *from, const char *to);
void imc_recv_keepalive(const char *from, const char *version, const char *flags);
void imc_recv_ping(const char *from, int time_s, int time_u, const char *path);
void imc_recv_pingreply(const char *from, int time_s, int time_u, const char *pathto, const char *pathfrom);
void imc_recv_whois(const imc_char_data *from, const char *to);
void imc_recv_whoisreply(const char *to, const char *text);
void imc_recv_inforequest(const char *from);

void imc_traceroute(int ping, const char *pathto, const char *pathfrom);

/* other functions */
const char *imc_sockets(void);
const char *imc_getstats(void);


/* imc-config.c exported functions */

/* ignore handling */
imc_ignore_data *imc_findignore(const char *who, int type);
int imc_isignored(const char *who);
void imc_addignore(const char *who, int flags);
int imc_delignore(const char *who);
imc_ignore_data *imc_newignore(void);
void imc_freeignore(imc_ignore_data *ign);

const char *imc_ignore(const char *what);
const char *imc_list(int level);
int imc_command(const char *argument);
int imc_saveconfig(void);
int imc_readconfig(void);
int imc_saveignores(void);
int imc_readignores(void);

int imc_readconfighook(const char *type, const char *value);
void imc_saveconfighook(FILE *fp);

/* Globals for optimization routines. I'm sure there's a better way to handle this,
but I'll save that for version 1.1, thank you very much. 1/6/99 -- Scion */
extern char global_hubname[IMC_MNAME_LENGTH];
extern int global_directnum;





/* imc-mail.c exported functions */

void imc_recv_mailok(const char *from, const char *id);
void imc_recv_mailrej(const char *from, const char *id, const char *reason);
void imc_recv_mail(const char *from, const char *to, const char *date,
		   const char *subject, const char *id, const char *text);
void imc_send_mail(const char *from, const char *to, const char *date,
		   const char *subject, const char *text);

char *imc_mail_arrived(const char *from, const char *to, const char *date,
		       const char *subject, const char *text);

void imc_mail_startup(void);
void imc_mail_shutdown(void);

char *imc_mail_showqueue(void);

#endif
