/*
                     R E A L M S    O F    D E S P A I R  !
   ___________________________________________________________________________
  //            /                                                            \\
 [|_____________\   ********   *        *   ********   *        *   *******   |]
 [|   \\._.//   /  **********  **      **  **********  **      **  *********  |]
 [|   (0...0)   \  **********  ***    ***  **********  ***    ***  *********  |]
 [|    ).:.(    /  ***         ****  ****  ***    ***  ***    ***  ***        |]
 [|    {o o}    \  *********   **********  **********  ***    ***  *** ****   |]
 [|   / ' ' \   /   *********  *** ** ***  **********  ***    ***  ***  ****  |]
 [|-'- /   \ -`-\         ***  ***    ***  ***    ***  ***    ***  ***   ***  |]
 [|   .VxvxV.   /   *********  ***    ***  ***    ***  **********  *********  |]
 [|_____________\  **********  **      **  **      **  **********  *********  |]
 [|             /  *********   *        *  *        *   ********    *******   |]
  \\____________\____________________________________________________________//
     |                                                                     |
     |    --{ [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame }--    |
     |_____________________________________________________________________|
     |                                                                     |
     |                      -*- Web Server Module -*-                      |
     |_____________________________________________________________________|
    //                                                                     \\
   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
   [|                                                                       |]
   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
   [|  Nyboe. Win32 port Nick Gammon.                                       |]
   [|                                                                       |]
   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
    \\_____________________________________________________________________//
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>

#include "mud.h"

#ifdef WEBSRV

#include "websrv.h"

void init_web(int webport) {
    struct sockaddr_in my_addr;

    web_descs = NULL;

    sprintf (log_buf, "Web features starting on port: %d", webport);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("web-socket");
	exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(webport);
    my_addr.sin_addr.s_addr = htons(INADDR_ANY);
    bzero(&(my_addr.sin_zero),8);

    if((bind(sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr))) == -1)
    {
	perror("web-bind");
	exit(1);
    }

    /* Only listen for 5 connects at once, do we really need more? */
    listen(sockfd, 5);
}

struct timeval ZERO_TIME = { 0, 0 };

void handle_web(void) {
	int max_fd;
	WEB_DESCRIPTOR *current, *prev, *next;
	fd_set readfds;

	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);

	/* it *will* be atleast sockfd */
	max_fd = sockfd;

	/* add in all the current web descriptors */
	for(current=web_descs; current != NULL; current = current->next) {
	    FD_SET(current->fd, &readfds);
	    if(max_fd < current->fd)
		max_fd = current->fd;
	}
	
	/* Wait for ONE descriptor to have activity */
	select(max_fd+1, &readfds, NULL, NULL, &ZERO_TIME);

	if(FD_ISSET(sockfd, &readfds)) {
            /* NEW CONNECTION -- INIT & ADD TO LIST */

	    current = new_web_desc();
	    current->sin_size  = sizeof(struct sockaddr_in);
	    current->request[0] = '\0';

	    if((current->fd = accept(sockfd, (struct sockaddr *)&(current->their_addr), &(current->sin_size))) == -1) {
	    	perror("web-accept");
	    	exit(1);
	    }

	    current->next = web_descs;
	    web_descs = current;

	    /* END ADDING NEW DESC */
	}

	/* DATA IN! */
	for(current=web_descs; current != NULL; current = current->next) {
	    if (FD_ISSET(current->fd, &readfds)) /* We Got Data! */
	    {
	    	char buf[MAXDATA];
		int numbytes;

		if((numbytes=read(current->fd,buf,sizeof(buf))) == -1) {
		    perror("web-read");
		    exit(1);
		}

		buf[numbytes] = '\0';

		strcat(current->request,buf);
	    }
	} /* DONE WITH DATA IN */

	/* DATA OUT */
	for(current=web_descs; current != NULL; current = next ){
	    next = current->next;

	    if(strstr(current->request, "HTTP/1.") /* 1.x request (vernum on FIRST LINE) */
	    && strstr(current->request, ENDREQUEST))
		handle_web_request(current);
	    else if(!strstr(current->request, "HTTP/1.")
		 &&  strchr(current->request, '\n')) /* HTTP/0.9 (no ver number) */
		handle_web_request(current);		
	    else {
		continue; /* Don't have full request yet! */
	    }

	    close(current->fd);

	    if(web_descs == current) {
		web_descs = current->next;
	    } else {
		for(prev=web_descs; prev->next != current; prev = prev->next)
			; /* Just ititerate through the list */
		prev->next = current->next;
	    }

	    free_web_desc(current);
	}   /* END DATA-OUT */
}

/* Generic Utility Function */

int send_buf(int fd, const char* buf) {
	return send(fd, buf, strlen(buf), 0);
}

void handle_web_request(WEB_DESCRIPTOR *wdesc) {
	    /* process request */
	    /* are we using HTTP/1.x? If so, write out header stuff.. */
	    if(!strstr(wdesc->request, "GET")) {
		send_buf(wdesc->fd,"HTTP/1.0 501 Not Implemented");
		return;
	    } else if(strstr(wdesc->request, "HTTP/1.")) {
		send_buf(wdesc->fd,"HTTP/1.0 200 OK\n");
		send_buf(wdesc->fd,"Content-type: text/html\n\n");
	    }

	    /* Handle the actual request */
	    if(strstr(wdesc->request, "/wholist")) {
				log_string("Web Hit: WHOLIST");
				handle_web_who_request(wdesc);
	    } else {
				log_string("Web Hit: INVALID URL");
				send_buf(wdesc->fd,"Sorry, --{SMAUG}-- Integrated Webserver 1.0 only supports /wholist");
	    }
}

void shutdown_web (void) {
    WEB_DESCRIPTOR *current,*next;

    /* Close All Current Connections */
    for(current=web_descs; current != NULL; current = next) {
	next = current->next;
	close(current->fd);
	free_web_desc(current);
    }

    /* Stop Listening */
    close(sockfd);
}

void handle_web_who_request(WEB_DESCRIPTOR *wdesc)
{
  CHAR_DATA *victim;
  int count=0;
  char output[MAX_STRING_LENGTH];
  char *clan_name,wizi_string[MAX_STRING_LENGTH];
  const char *class;
  int legend=95,hero=93,dragon=89,master=79,lord=69,duke=59, \
      leader=49,adven=39,explo=29,student=19,train=9;
  DESCRIPTOR_DATA *d;
  DESCRIPTOR_DATA *descriptor_list;


  send_buf(wdesc->fd,"<HTML><HEAD><TITLE>Realms of Despair - Who List - </TITLE></HEAD>\n\r");
  send_buf(wdesc->fd,"<BODY BGCOLOR=\"#FFFFFF\"><B>Realms of Despair - Who List - </B><P>\n\r");

  for (d = descriptor_list; d; d = d->next)
  {
    victim = ( d->original ) ? d->original : d->character;

    if (d->connected != CON_PLAYING || victim->mobinvis > 0 ||
        IS_NPC(victim)) {
      continue;
    }

    count++;
            if (victim->level == MAX_LEVEL)        class = "--OVERLORD--";
        else if (victim->level >= LEVEL_IMMORTAL)  class = " -IMMORTAL- ";
                else if (victim->level >= legend)  class = "<--LEGEND-->";
            else if (victim->level >= hero)        class = "<-->HERO<-->";
            else if (victim->level >= dragon)      class = ">DRAGONLORD<";
            else if (victim->level >= master)      class = "<<<MASTER>>>";
           else if (victim->level >= lord)         class = " ***LORD*** ";
            else if (victim->level >= duke)        class = " *-*DUKE*-* ";
            else if (victim->level >= leader)      class = " **LEADER** ";
            else if (victim->level >= adven)       class = "-ADVENTURER-";
            else if (victim->level >= explo)       class = "  EXPLORER  ";
            else if (victim->level >= student)     class = " -StUdEnT-  ";
            else if (victim->level >= train)       class = "  TRAINEE  ";
            else if (victim->level < train)        class = ">> NEWBIE <<";
            else                                class = "   ERROR    ";

    sprintf (wizi_string,"(W:%d) ", victim->mobinvis);

    if ( is_clan(victim) )
        clan_name = str_dup( victim->pcdata->clan->name );
    else
        clan_name = str_dup( "" );

    sprintf(output, "%s[%s] %s%s%s%s%s%s%s%s<BR>",
	    IS_IMMORTAL(victim) ? "<B>" : "",
            class,
            victim->mobinvis >= LEVEL_HERO ? wizi_string : "",
            clan_name,
            IS_SET(victim->pcdata->flags, PLR_AFK) ? "[AFK] " : "",
            d->connected >= CON_PLAYING ? "[PLAYING] " : "",
            victim->name, victim->pcdata->title,
	    IS_IMMORTAL(victim) ? "</B>" : "");
      send_buf(wdesc->fd,output);
  }
  sprintf(output, "<P>--{SMAUG}-- Who-List [%d players found]</BODY></HTML>", count);
  send_buf(wdesc->fd,output);
}

/* These are memory management... they should move to recycle.c soon */

WEB_DESCRIPTOR *new_web_desc(void)
{
		int top_web_desc;
    WEB_DESCRIPTOR *desc;

    if(web_desc_free == NULL) {
	desc = pvalloc(sizeof(*desc));
	top_web_desc++;
    } else {
	desc = web_desc_free;
	web_desc_free = web_desc_free->next;
    }

    is_valid_tag(desc);

    return desc;	
}

void free_web_desc(WEB_DESCRIPTOR *desc)
{
    if(!is_valid_tag(desc))
	return;

    // INVALIDATE(desc);
    desc->next = web_desc_free;
    web_desc_free = desc;
}

#endif
