/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-mail.c: IMC mailer functions
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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "imc.h"

/*
 *  general stuff
 */

/* escape a string for writing to a file */

static const char *escape(const char *data)
{
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  char *p;

  for (p=buf; *data && (p-buf < IMC_DATA_LENGTH-1); data++, p++)
  {
    if (*data == '\n')
    {
      *p++='\\';
      *p='n';
    }
    else if (*data == '\r')
    {
      *p++='\\';
      *p='r';
    }
    else if (*data == '\\')
    {
      *p++='\\';
      *p='\\';
    }
    else if (*data == '"')
    {
      *p++='\\';
      *p='"';
    }
    else
      *p=*data;
  }

  *p=0;

  imc_shrinksbuf(buf);
  return buf;
}

/* unescape: reverse escape */
static const char *unescape(const char *data)
{
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  char *p;
  char ch;

  for (p=buf; *data && (p-buf < IMC_DATA_LENGTH-1); data++, p++)
  {
    if (*data == '\\')
    {
      ch = *(++data);
      switch (ch)
      {
      case 'n':
	*p='\n';
	break;
      case 'r':
	*p='\r';
	break;
      case '\\':
	*p='\\';
	break;
      default:
	*p=ch;
	break;
      }
    }
    else
      *p=*data;
  }

  *p=0;

  imc_shrinksbuf(buf);
  return buf;
}



/*
 *  mail
 */

/* new_mail: create a new mail structure */
static imc_mail *new_mail(void)
{
  imc_mail *p;

  p=imc_malloc(sizeof(*p));

  p->from     = NULL;
  p->to       = NULL;
  p->subject  = NULL;
  p->date     = NULL;
  p->text     = NULL;
  p->id       = NULL;
  p->received = 0;
  p->usage    = 0;
  p->next     = NULL;

  return p;
}

/* free_mail: free a mail structure */
static void free_mail(imc_mail *p)
{
  if (!p)
  {
    imc_logerror("BUG: free_mail: freeing NULL pointer");
    return;
  }

  if (p->usage)
  {
    imc_logerror("BUG: free_mail: freeing mail at %p with usage=%d",
		 p, p->usage);
    return;
  }

  if (p->from)
    imc_strfree(p->from);
  if (p->to)
    imc_strfree(p->to);
  if (p->id)
    imc_strfree(p->id);
  if (p->text)
    imc_strfree(p->text);
  if (p->subject)
    imc_strfree(p->subject);
  if (p->date)
    imc_strfree(p->date);

  imc_cancel_event(NULL, p);

  imc_free(p, sizeof(*p));
}

/* write_mail: write a single mail to a file */
static void write_mail(imc_mail *p, FILE *out)
{
  if (!p)
  {
    imc_logerror("BUG: write_mail: NULL pointer");
    return;
  }

  fprintf(out, "From %s\n", escape(p->from));
  fprintf(out, "To %s\n", escape(p->to));
  fprintf(out, "Subject %s\n", escape(p->subject));
  fprintf(out, "Date %s\n", escape(p->date));
  fprintf(out, "Text %s\n", escape(p->text));
  fprintf(out, "ID %s\n", escape(p->id));
  fprintf(out, "Received %ld\n", p->received);
}

/* read_mail: read a single mail from a file, NULL on EOF */
static imc_mail *read_mail(FILE *in)
{
  imc_mail *p;
  char line[IMC_DATA_LENGTH];
  char temp[IMC_DATA_LENGTH];

  fgets(line, IMC_DATA_LENGTH, in);
  if (ferror(in) || feof(in))
    return NULL;

  p=new_mail();

  sscanf(line, "From %[^\n]", temp);
  p->from=imc_strdup(unescape(temp));
  fgets(line, IMC_DATA_LENGTH, in);
  sscanf(line, "To %[^\n]", temp);
  p->to=imc_strdup(unescape(temp));
  fgets(line, IMC_DATA_LENGTH, in);
  sscanf(line, "Subject %[^\n]", temp);
  p->subject=imc_strdup(unescape(temp));
  fgets(line, IMC_DATA_LENGTH, in);
  sscanf(line, "Date %[^\n]", temp);
  p->date=imc_strdup(unescape(temp));
  fgets(line, IMC_DATA_LENGTH, in);
  sscanf(line, "Text %[^\n]", temp);
  p->text=imc_strdup(unescape(temp));
  fgets(line, IMC_DATA_LENGTH, in);
  sscanf(line, "ID %[^\n]", temp);
  p->id=imc_strdup(unescape(temp));
  fgets(line, IMC_DATA_LENGTH, in);
  sscanf(line, "Received %ld", &p->received);

  p->usage=0;

  return p;
}


/*
 *  maillist - a list of imc_mail entries
 */

imc_mail *imc_ml_head;

/* init_ml: init the maillist */
static void init_ml(void)
{
  imc_ml_head          = new_mail();
  imc_ml_head->to      = imc_strdup("");
  imc_ml_head->from    = imc_strdup("");
  imc_ml_head->date    = imc_strdup("");
  imc_ml_head->subject = imc_strdup("");
  imc_ml_head->text    = imc_strdup("");
  imc_ml_head->id      = imc_strdup("");
}

/* free_ml: free the maillist */

static void free_ml(void)
{
  imc_mail *p, *p_next;

  for (p=imc_ml_head; p; p=p_next)
  {
    p_next=p->next;
    p->usage=0; /* suppress warnings */
    free_mail(p);
  }

  imc_ml_head=NULL;
}

/* add_ml: add an item to the maillist */

static void add_ml(imc_mail *p)
{
  if (!p)
  {
    imc_logerror("BUG: add_ml: adding NULL pointer");
    return;
  }

  p->next=imc_ml_head->next;
  imc_ml_head->next=p;
}

/* find_ml: find a given ID in the mail list */

static imc_mail *find_ml(const char *id)
{
  imc_mail *p;

  if (!id)
  {
    imc_logerror("BUG: find_ml: NULL id");
    return NULL;
  }

  for (p=imc_ml_head->next; p; p=p->next)
    if (!strcasecmp(p->id, id))
      return p;

  return NULL;
}

/* delete_ml: delete a given node in the mail list */

static void delete_ml(imc_mail *node)
{
  imc_mail *last, *p;

  if (!node)
  {
    imc_logerror("BUG: delete_ml: NULL node");
    return;
  }

  for (last=imc_ml_head, p=last->next; p && p != node; p=p->next)
    ;

  if (p)
  {
    last->next = p->next;
    free_mail(p);
  }
  else
    imc_logerror("BUG: delete_ml: node at %p not on list", node);
}

/* save_ml: save maillist */
static void save_ml(void)
{
  FILE *out;
  imc_mail *p;
  char name[200];

  imc_sncpy(name, imc_prefix, 190);
  strcat(name, "mail-list");

  out=fopen(name, "w");
  if (!out)
  {
    imc_lerror("save_ml: fopen");
    return;
  }

  for (p=imc_ml_head->next; p; p=p->next)
    write_mail(p, out);

  fclose(out);
}

/* load_ml: load maillist, assumes init_ml done */
static void load_ml(void)
{
  FILE *in;
  imc_mail *p;
  char name[200];

  imc_sncpy(name, imc_prefix, 190);
  strcat(name, "mail-list");

  in=fopen(name, "r");
  if (!in)
    return;

  p=read_mail(in);
  while (p)
  {
    add_ml(p);
    p=read_mail(in);
  }

  fclose(in);
}




/*
 *  qnode - an entry in the 'mail to send' queue, referencing a particular
 *          piece of mail, and the mud that this entry needs to send to
 */

/* new_qnode: get a new qnode */
static imc_qnode *new_qnode(void)
{
  imc_qnode *p;

  p=imc_malloc(sizeof(*p));

  p->data  = NULL;
  p->next  = NULL;
  p->tomud = NULL;

  return p;
}

/* free_qnode: free a qnode */
static void free_qnode(imc_qnode *q)
{
  if (!q)
  {
    imc_logerror("BUG: free_qnode: freeing NULL pointer");
    return;
  }

  if (q->tomud)
    imc_strfree(q->tomud);

  if (q->data && !--q->data->usage)
    delete_ml(q->data);

  imc_cancel_event(NULL, q);

  imc_free(q, sizeof(*q));
}

/* write_qnode: write a qnode to a file */
static void write_qnode(imc_qnode *q, FILE *out)
{
  if (!q)
  {
    imc_logerror("BUG: write_qnode: NULL pointer");
    return;
  }

  fprintf(out, "%s %s\n", q->data->id, q->tomud);
}

/* read_qnode: read a qnode from a file */
static imc_qnode *read_qnode(FILE *in)
{
  imc_qnode *p;
  imc_mail *m;
  char line[IMC_DATA_LENGTH];
  char temp1[IMC_DATA_LENGTH], temp2[IMC_DATA_LENGTH];

  fgets(line, IMC_DATA_LENGTH, in);
  if (ferror(in) || feof(in))
    return NULL;

  sscanf(line, "%[^ ] %[^\n]", temp1, temp2);
  m=find_ml(temp1);
  if (!m)
  {
    imc_logerror("read_qnode: ID %s not in mail queue", temp1);
    return NULL;
  }

  p=new_qnode();

  m->usage++;
  p->data  = m;
  p->tomud = imc_strdup(temp2);

  return p;
}


/*
 *  mailqueue - a list of active qnodes
 */

imc_qnode *imc_mq_head, *imc_mq_tail;

/* init_mq: init mailqueue */
static void init_mq(void)
{
  imc_mq_head       = new_qnode();
  imc_mq_tail       = imc_mq_head;
  imc_mq_head->data = NULL;
  imc_mq_head->next = NULL;
}

/* free_mq: delete mailqueue */
static void free_mq(void)
{
  imc_qnode *p, *p_next;

  for (p=imc_mq_head; p; p=p_next)
  {
    p_next=p->next;
    free_qnode(p);
  }

  imc_mq_head=imc_mq_tail=NULL;
}

/* add_mq: add a queue of items to the tail of the mq */
static void add_mq(imc_qnode *p)
{
  imc_mq_tail->next=p;
  while (p->next)
    p=p->next;
  imc_mq_tail=p;
}

#if 0
/* get_mq: extract the head of the mail queue */
static imc_qnode *get_mq(void)
{
  imc_qnode *p;

  if (imc_mq_head == imc_mq_tail)	/* empty queue */
    return NULL;

  p=imc_mq_head->next;
  imc_mq_head->next=p->next;
  if (p == imc_mq_tail)
    imc_mq_tail=imc_mq_head;

  p->next=NULL;	                        /* Just In Case */

  return p;
}
#endif

/* find_mq: find the item with the given ID/tomud values */
static imc_qnode *find_mq(const char *id, const char *tomud)
{
  imc_qnode *p;

  for (p=imc_mq_head->next; p; p=p->next)
    if (!strcmp(id, p->data->id) && !strcasecmp(tomud, p->tomud))
      return p;

  return NULL;
}

/* delete_mq: delete the item with the given ID/tomud values */
static void delete_mq(const char *id, const char *tomud)
{
  imc_qnode *p, *last;

  for (last=imc_mq_head, p=last->next; p; last=p, p=p->next)
    if (!strcmp(id, p->data->id) && !strcasecmp(tomud, p->tomud))
    {
      last->next=p->next;
      if (p == imc_mq_tail)
	imc_mq_tail=last;
      free_qnode(p);
      return;
    }
}

/* save mailqueue */
static void save_mq(void)
{
  FILE *out;
  imc_qnode *p;
  char name[200];

  imc_sncpy(name, imc_prefix, 189);
  strcat(name, "mail-queue");

  out=fopen(name, "w");
  if (!out)
  {
    imc_lerror("save_mq: fopen");
    return;
  }

  for (p=imc_mq_head->next; p; p=p->next)
    write_qnode(p, out);

  fclose(out);
}

/* load mailqueue, assumes init_mq done */
static void load_mq(void)
{
  FILE *in;
  imc_qnode *p;
  char name[200];
  int when=10;

  imc_sncpy(name, imc_prefix, 189);
  strcat(name, "mail-queue");

  in=fopen(name, "r");
  if (!in)
    return;

  p=read_qnode(in);
  while (!feof(in) && !ferror(in))
  {
    if (p)
    {
      add_mq(p);
      imc_add_event(when, ev_qnode_send, p, 1);
      when += rand()%30+30;
    }
    p=read_qnode(in);
  }

  fclose(in);
}




/*
 *  mailid - a single mail ID that has been received
 */

/* new_mailid: get a new mailid */

static imc_mailid *new_mailid(void)
{
  imc_mailid *p;

  p=imc_malloc(sizeof(*p));

  p->id       = NULL;
  p->received = 0;
  p->next     = NULL;

  return p;
}

/* free_mailid: free a mailid */
static void free_mailid(imc_mailid *p)
{
  if (!p)
  {
    imc_logerror("BUG: free_mailid: freeing NULL pointer");
    return;
  }

  if (p->id)
    imc_strfree(p->id);

  imc_cancel_event(NULL, p);

  imc_free(p, sizeof(*p));
}

/* generate_mailid: generate a new mailid (string) */
static char *generate_mailid(void)
{
  char *buffer=imc_getsbuf(200);

  sprintf(buffer, "%d-%ld@%s", rand(), imc_sequencenumber++, imc_name);
  imc_shrinksbuf(buffer);
  return buffer;
}

/* write_mailid: write a mailid to a file */
static void write_mailid(imc_mailid *p, FILE * out)
{
  fprintf(out, "%s %ld\n", p->id, p->received);
}

/* read_mailid: read a mailid from a file, NULL on EOF */
static imc_mailid *read_mailid(FILE *in)
{
  imc_mailid *p;
  char line[IMC_DATA_LENGTH];
  char temp[IMC_DATA_LENGTH];
  time_t r;

  fgets(line, IMC_DATA_LENGTH, in);
  if (ferror(in) || feof(in))
    return NULL;

  sscanf(line, "%[^ ] %ld", temp, &r);

  p=new_mailid();

  p->id       = imc_strdup(temp);
  p->received = r;

  return p;
}



/*
 *  idlist - a list of mail IDs received over the last 24 hours
 */

imc_mailid *imc_idlist;

/* init_idlist: init the ID list */
static void init_idlist(void)
{
  imc_idlist=new_mailid();
}

/* free_idlist: free the idlist */
static void free_idlist(void)
{
  imc_mailid *p, *p_next;

  for (p=imc_idlist; p; p=p_next)
  {
    p_next=p->next;
    free_mailid(p);
  }

  imc_idlist=NULL;
}

/* add_idlist: add an ID to the idlist */

static void add_idlist(imc_mailid *p)
{
  p->next=imc_idlist->next;
  imc_idlist->next=p;
}

/* find_id: check if an ID is in the ID list */

static imc_mailid *find_id(const char *id)
{
  imc_mailid *p;

  for (p=imc_idlist->next; p; p=p->next)
    if (!strcmp(p->id, id))
      return p;

  return NULL;
}

/* flush_idlist: flush old entries from the mailseen list */
static void flush_idlist(time_t at)
{
  imc_mailid *p, *last;

  for (last=imc_idlist, p=last->next; p; p=p->next)
    if (p->received < at)	/* delete this entry */
    {
      last->next=p->next;
      free_mailid(p);
      p=last;
    }
}

/* save_idlist: save idlist */
static void save_idlist(void)
{
  FILE *out;
  imc_mailid *p;
  char name[200];

  imc_sncpy(name, imc_prefix, 191);
  strcat(name, "mail-ids");

  out=fopen(name, "w");
  if (!out)
  {
    imc_lerror("save_idlist: fopen");
    return;
  }

  for (p=imc_idlist->next; p; p=p->next)
    write_mailid(p, out);

  fclose(out);
}

/* load_idlist: load idlist, assumes init_idlist done */
static void load_idlist(void)
{
  FILE *in;
  imc_mailid *p;
  char name[200];

  imc_sncpy(name, imc_prefix, 191);
  strcat(name, "mail-ids");

  in=fopen(name, "r");
  if (!in)
    return;

  p=read_mailid(in);
  while (p)
  {
    add_idlist(p);
    p=read_mailid(in);
  }

  fclose(in);
}

/* datestring: generate a date string for the current time */
static char *datestring(void)
{
  char *buf=imc_getsbuf(100);

  strcpy(buf, ctime(&imc_now));
  buf[strlen(buf)-1]=0;

  imc_shrinksbuf(buf);
  return buf;
}

/* bounce: generate a local bounce note */

static void bounce(imc_mail *item, const char *source, const char *reason)
{
  char temp[IMC_DATA_LENGTH];

  sprintf(temp,
	  "Your mail of %s:\n\r"
	  " to: %s\n\r"
	  " re: %s\n\r"
	  "was undeliverable for the following reason:\n\r"
	  "\n\r"
	  "%s: %s\n\r",
	  item->date,
	  item->to,
	  item->subject,
	  source,
	  reason);

  imc_mail_arrived("Mail-daemon", imc_nameof(item->from), datestring(),
		   "Bounced mail", temp);
}

/* expire old entries in the mailid list; called once an hour */
void ev_mailid_expire(void *data)
{
  flush_idlist(imc_now + 24*3600);
  imc_add_event(3600, ev_mailid_expire, NULL, 1);
}

/* give up sending a given qnode */
void ev_qnode_expire(void *data)
{
  char temp[200];
  imc_qnode *p=(imc_qnode *)data;

  sprintf(temp, "Unable to send to %s after 12 hours, giving up", p->tomud);
  bounce(p->data, imc_name, temp);
  delete_mq(p->data->id, p->tomud);
  save_ml();
  save_mq();
}

/* try sending a qnode */
void ev_qnode_send(void *data)
{
  imc_qnode *p=(imc_qnode *)data;
  imc_packet out;

  save_ml();
  save_mq();

  /* send it.. */

  imc_initdata(&out.data);

  sprintf(out.to, "Mail-daemon@%s", p->tomud);
  strcpy(out.from, "Mail-daemon");
  strcpy(out.type, "mail");

  imc_addkey(&out.data, "to", p->data->to);
  imc_addkey(&out.data, "from", p->data->from);
  imc_addkey(&out.data, "subject", p->data->subject);
  imc_addkey(&out.data, "date", p->data->date);
  imc_addkey(&out.data, "text", p->data->text);
  imc_addkey(&out.data, "id", p->data->id);

  imc_send(&out);
  imc_freedata(&out.data);

  /* try resending it in an hour */

  imc_add_event(3600, ev_qnode_send, data, 1);
}

/* imc_recv_mailok: a mail-ok packet was received */
void imc_recv_mailok(const char *from, const char *id)
{
  delete_mq(id, imc_mudof(from));
  save_mq();
  save_ml();	       	/* we might have removed the mail if usage==0 */
}

/* imc_recv_mailrej: a mail-reject packet was received */
void imc_recv_mailrej(const char *from, const char *id, const char *reason)
{
  imc_qnode *p;

  p = find_mq(id, imc_mudof(from));
  if (!p)
    return;

  bounce(p->data, from, reason);

  delete_mq(id, imc_mudof(from));
  save_mq();
  save_ml();
}

/* addrtomud: convert a 'to' list to the local mud format (ie strip @mudname
 * when it matches imc_name)
 */

static void addrtomud(const char *list, char *output)
{
  char arg[IMC_NAME_LENGTH];

  output[0]=0;
  list=imc_getarg(list, arg, IMC_NAME_LENGTH);

  while (*arg)
  {
    if (!strcasecmp(imc_name, imc_mudof(arg)))
      sprintf(output + strlen(output), "%s ", imc_nameof(arg));
    else
      sprintf(output + strlen(output), "%s ", arg);

    list=imc_getarg(list, arg, IMC_NAME_LENGTH);
  }
}

/* mudtoaddr: add the @mudname to a 'to' list for unqualified names */

static void mudtoaddr(const char *list, char *output)
{
  char arg[IMC_NAME_LENGTH];

  output[0]=0;

  list=imc_getarg(list, arg, IMC_NAME_LENGTH);

  while (*arg)
  {
    if (strchr(arg, '@') == NULL)
      sprintf(output + strlen(output), "%s@%s ", arg, imc_name);
    else
      sprintf(output + strlen(output), "%s ", arg);

    list=imc_getarg(list, arg, IMC_NAME_LENGTH);
  }

  /* chop final space */

  if (arg[0] && arg[strlen(arg) - 1] == ' ')
    arg[strlen(arg) - 1] = 0;
}

/* imc_recv_mail: a mail packet was received */

void imc_recv_mail(const char *from, const char *to, const char *date,
		   const char *subject, const char *id, const char *text)
{
  imc_mailid *mid;
  imc_packet out;
  char *reason;
  char temp[IMC_DATA_LENGTH];

  imc_initdata(&out.data);
  sprintf(out.to, "Mail-daemon@%s", imc_mudof(from));
  strcpy(out.from, "Mail-daemon");

  /* check if we've already seen it */

  mid=find_id(id);

  if (mid)
  {
    strcpy(out.type, "mail-ok");
    imc_addkey(&out.data, "id", id);

    imc_send(&out);
    imc_freedata(&out.data);

    mid->received = imc_now;

    return;
  }

  /* check for rignores */

  if (imc_isignored(from))
  {
    strcpy(out.type, "mail-reject");
    imc_addkey(&out.data, "id", id);
    imc_addkey(&out.data, "reason", "You are being ignored.");

    imc_send(&out);
    imc_freedata(&out.data);
    return;
  }

  /* forward it to the mud */

  addrtomud(to, temp);

  if ((reason=imc_mail_arrived(from, temp, date, subject, text)) == NULL)
  {
    /* it was OK */

    strcpy(out.type, "mail-ok");
    imc_addkey(&out.data, "id", id);

    imc_send(&out);
    imc_freedata(&out.data);

    mid=new_mailid();
    mid->id=imc_strdup(id);
    mid->received=imc_now;

    add_idlist(mid);
    save_idlist();
    return;
  }

  /* mud rejected the mail */

  strcpy(out.type, "mail-reject");
  imc_addkey(&out.data, "id", id);
  imc_addkey(&out.data, "reason", reason);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* imc_send_mail: called by the mud to add a piece of mail to the queue */
void imc_send_mail(const char *from, const char *to, const char *date,
		   const char *subject, const char *text)
{
  char temp[IMC_DATA_LENGTH];
  imc_mail *m;
  imc_qnode *qroot, *q;
  char arg[IMC_NAME_LENGTH];
  const char *mud;
  int when=10;

  /* set up the entry for the mail list */

  m=new_mail();

  mudtoaddr(to, temp);		/* qualify local addresses */
  m->to       = imc_strdup(temp);
  sprintf(temp, "%s@%s", from, imc_name);	/* qualify sender */
  m->from     = imc_strdup(temp);
  m->date     = imc_strdup(date);
  m->subject  = imc_strdup(subject);
  m->id       = imc_strdup(generate_mailid());
  m->text     = imc_strdup(text);
  m->received = imc_now;

  qroot=NULL;			/* initialise the local list */
  to=imc_getarg(to, arg, IMC_NAME_LENGTH);

  while (*arg)
  {
    /*  get a mudname and check if we've already added a queue entry for that
     *  mud. If not, add it
     */

    if (strchr(arg, '@') != NULL && (mud = imc_mudof(arg))[0] != 0 &&
	strcasecmp(mud, imc_name))
    {
      if (!strcmp(mud, "*"))
	q=NULL;	                /* catch the @* case - not yet implemented */
      else
	for (q=qroot; q; q=q->next)
	  if (!strcasecmp(q->tomud, mud))
	    break;

      if (!q)			/* not seen yet */
      {				/* add to the top of our mini-queue */
	q=new_qnode();
	q->tomud=imc_strdup(mud);
	q->data=m;
	q->next=qroot;
	m->usage++;
	qroot=q;

	imc_add_event(when, ev_qnode_send, q, 1);
	when += rand()%30+30;
      }
    }

    /* get the next address */

    to=imc_getarg(to, arg, IMC_NAME_LENGTH);
  }

  if (!qroot)			/* boggle, no foreign addresses?? */
  {
    free_mail(m);
    return;
  }

  /* add mail to mail list, add mini-queue to mail queue */

  add_ml(m);
  add_mq(qroot);
  save_ml();
  save_mq();
}

/* imc_mail_startup: start up the mail subsystem */
void imc_mail_startup(void)
{
  init_mq();
  init_ml();
  init_idlist();

  /* order is important here: we need the maillist to resolve the ID refs in
   * the mailqueue
   */

  load_ml();
  load_mq();
  load_idlist();

  /* queue an expiry event */

  imc_add_event(24*3600, ev_mailid_expire, NULL, 0);
}

/* imc_mail_shutdown: shut down the mailer */
void imc_mail_shutdown(void)
{
  save_mq();
  save_ml();
  save_idlist();

  free_mq();
  free_ml();
  free_idlist();

  imc_cancel_event(ev_mailid_expire, NULL);
}

/* imc_mail_showqueue: returns the current mail queue
 * buffer handling here is pretty ugly, oh well
 */
char *imc_mail_showqueue(void)
{
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  char temp[100];
  imc_qnode *p;
  int left = IMC_DATA_LENGTH;

  sprintf(buf, "%-15s %-45s %-10s %s\n\r", "From", "To", "Via", "Time");

  for (p=imc_mq_head->next; p && left > 160; p = p->next)
  {
    int m, s;

    m=imc_next_event(ev_qnode_send, p);
    if (m<0)
      sprintf(temp, "%-15.15s %-45.45s %-10.10s --:--\n\r",
	      p->data->from,
	      p->data->to,
	      p->tomud);
    else      
    {
      s=m%60;
      m/=60;
      sprintf(temp, "%-15.15s %-45.45s %-10.10s %2d:%02d\n\r",
	      p->data->from,
	      p->data->to,
	      p->tomud,
	      m, s);
    }
	      
    left -= strlen(temp);
    strcat(buf, temp);
  }

  if (p)
  {
    int count;

    for (count=0; p; p=p->next, count++)
      ;

    sprintf(temp, "[%d further entries omitted]\n\r", count);
    strcat(buf, temp);
  }

  imc_shrinksbuf(buf);
  return buf;
}
