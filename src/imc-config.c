/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-config.c: configuration manipulation
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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#include "imc.h"
int imc_log_on=1;
int imc_hubswitch=1;
extern time_t imc_boot;
/*
 * I needed to split up imc.c (2600 lines and counting..) so this file now
 * contains:
 *
 * - config loading/saving (imc_readconfig, imc_saveconfig)
 * - ignores loading/saving (imc_readignores, imc_saveignores)
 * - config editing/displaying (imc_command)
 * - ignore editing/displaying (imc_ignore)
 */

/*  USEIOCTL #defined if TIOCINQ or TIOCOUTQ are - we assume that the ioctls
 *  work in that case.
 */

#if defined(TIOCINQ) && defined(TIOCOUTQ)
#define USEIOCTL
static int outqsize;
#endif

#define GETSTRING(key,var,len)                                     \
  else if (!strcasecmp(word,key)) {                                \
  if (var[0]) imc_logstring("Warning: duplicate '" key "' lines"); \
  imc_sncpy(var,value,len);                                        \
  if(!strcasecmp(word,"InfoDetails")) strcat(var,"\n\r");          \
  }
/* append crlf for details - shogar */

imc_siteinfo_struct imc_siteinfo;

extern time_t time_since_last_ping; /* istat ping monitor - shogar */
extern time_t time_since_last_ralive; /* istat ping monitor - shogar */
extern time_t time_since_last_salive; /* istat ping monitor - shogar */

/* read config file */
int imc_readconfig(void)
{
  FILE *cf;
  imc_info *i;
  char name[IMC_NAME_LENGTH], host[200];
  char pw1[IMC_PW_LENGTH], pw2[IMC_PW_LENGTH];
  unsigned short port;
  int count;
  char buf[1000];
  char logon[10];
  char hubswitch[10];
  char configfile[200];
  int noforward, rcvstamp, flags;
  char word[1000];
  short localport=-1; /* added -1 to solve "duplicate localport" error. Suggested by Tagith@UCMM. -- Scion */
  char localname[IMC_NAME_LENGTH]; /* Again suggested ="" by Tagith@UCMM  -- Scion */
  const char *value;
  int version= -1;
  unsigned long localip;

char iname[200], ihost[200], iemail[200], iimail[200], iwww[200],
    idetails[4096], iflags[200],mguest[80];
  /* increased idetails - shogar */
  
  localport= -1;
  localname[0]=iname[0]=ihost[0]=iemail[0]=iimail[0]=iwww[0]=idetails[0]=
    iflags[0]=mguest[0]=logon[0]=hubswitch[0]=word[0]=0;
  localip=INADDR_ANY;
  
  imc_sncpy(configfile, imc_prefix, 193);
  strcat(configfile, "config");

  imc_info_list=NULL;

  cf=fopen(configfile, "r");
  if (!cf)
  {
    imc_logerror("imc_readconfig: couldn't open %s", configfile);
    return 0;
  }

  while(1)
  {
    if (fgets(buf, 1000, cf) == NULL)
      break;

    if(strlen(idetails) > 4096)
    {
	imc_logstring("Error: InfoDetails is too long");
	exit(1);
    }
    
    while (buf[0] && isspace(buf[strlen(buf)-1]))
      buf[strlen(buf)-1]=0;

    if (buf[0] == '#' || buf[0] == '\n' || !buf[0])
      continue;

    value=imc_getarg(buf, word, 1000);
    if (!strcasecmp(word, "Version"))
    {
      imc_getarg(value, word, 1000);
      version=atoi(word);
    }
    else if (!strcasecmp(word, "Connection"))
    {
      imc_getarg(value, word, 1000);
      if (sscanf(value, "%[^:]:%[^:]:%hu:%[^:]:%[^:]:%d:%d:%n",
		 name, host, &port, pw1, pw2,
		 &rcvstamp, &noforward, &count)<7)
      {
	imc_logerror("Bad 'Connection' line: %s", buf);
	continue;
      }

      flags=imc_flagvalue(word + count, imc_connection_flags);
      flags |= imc_flagvalue(value, imc_connection_flags); /* original bug - fix shogar */

      i=imc_new_info();
      i->name       = imc_strdup(name);
      i->host       = imc_strdup(host);
      i->clientpw   = imc_strdup(pw1);
      i->serverpw   = imc_strdup(pw2);
      i->port       = port;
      i->rcvstamp   = rcvstamp;
      i->noforward  = noforward;
      i->flags      = flags;
    }
    else if (!strcasecmp(word, "LocalPort"))
    {
      if (localport  > 0)
	imc_logstring("Warning: duplicate 'LocalPort' lines");
      localport=atoi(value);
    }
    else if (!strcasecmp(word, "Bind"))
    {
      if (localip!=INADDR_ANY)
	imc_logstring("Warning: duplicate 'Bind' lines");
      localip=inet_addr(value);
    }

    GETSTRING("LocalName", localname, 1000)
    GETSTRING("InfoName", iname, 200)
    GETSTRING("InfoHost", ihost, 200)
    GETSTRING("InfoEmail", iemail, 200)
    GETSTRING("InfoImail", iimail, 200)
    GETSTRING("InfoWWW", iwww, 200)
    GETSTRING("InfoFlags", iflags, 200)
/* allow multiple lines - shogar */
    GETSTRING("InfoDetails", (idetails + strlen(idetails)), 1000)
/* set max guest connections - shogar */
    GETSTRING("MaxGuests", mguest, 80)
  /* add configurable log message option - shogar */
    GETSTRING("LogOn", logon, 10)
    GETSTRING("HubSwitch",hubswitch , 10)
    
      
    else if (imc_readconfighook(word, value))
      ;
    else if (version==-1)
    {
      if (sscanf(buf, "%[^:]:%[^:]:%hu:%[^:]:%[^:]:%d:%d:%n",
		 name, host, &port, pw1, pw2,
		 &rcvstamp, &noforward, &count)<7)
      {
	imc_logerror("Bad config line: %s", buf);
	continue;
      }

      flags=imc_flagvalue(buf + count, imc_connection_flags);
    
      i=imc_new_info();
      i->name       = imc_strdup(name);
      i->host       = imc_strdup(host);
      i->clientpw   = imc_strdup(pw1);
      i->serverpw   = imc_strdup(pw2);
      i->port       = port;
      i->rcvstamp   = rcvstamp;
      i->noforward  = noforward;
      i->flags      = flags;
    }
    else
      imc_logerror("Bad config line: %s",
		   buf);
  }

  if (ferror(cf))
  {
    imc_lerror("imc_readconfig");
    fclose(cf);
    return 0;
  }

  fclose(cf);

  imc_siteinfo.name=imc_strdup(iname);
  imc_siteinfo.host=imc_strdup(ihost);
  imc_siteinfo.email=imc_strdup(iemail);
  imc_siteinfo.imail=imc_strdup(iimail);
  imc_siteinfo.www=imc_strdup(iwww);
  imc_siteinfo.details=imc_strdup(idetails);
  imc_siteinfo.flags=imc_strdup(iflags);
  if(imc_is_router)
  {
	if(! *mguest)
	{
		imc_logerror("MaxGuests is a required field");
		exit(1);
	}
	imc_siteinfo.maxguests=atoi(mguest);
  }
  
  if (!localname[0])
  {
    imc_logstring("warning: missing 'LocalName' line in config");
    imc_name=NULL;
  }
  else
    imc_name=imc_strdup(localname);

  if (localport==-1)
  {
    imc_logerror("warning: missing 'LocalPort' line in config");
    imc_port=0;
  }
  else
    imc_port=localport;

  imc_bind=localip;
  
  if (!iname[0] || !iemail[0] || !ihost[0])
  {
    imc_logerror("InfoName, InfoHost and InfoEmail MUST be set to start IMC. See the IMC README");
    exit(1);
  }
  /* add configurable log message option - shogar */
  if(!strcasecmp(logon,"no"))
  {
	imc_logstring("Logging Off");
	imc_log_on=0;
  }
  else if(!strcasecmp(logon,"yes"))
  {
	imc_logstring("Logging On");
	imc_log_on=1;
  }
  else
  {
	imc_logstring("Log yes|no");
  }
  /* add configurable hubswitching option - shogar */
  if(!strcasecmp(hubswitch,"no"))
  {
	imc_logstring("HubSwitch Off");
	imc_hubswitch=0;
  }
  else if(!strcasecmp(hubswitch,"yes"))
  {
	imc_logstring("HubSwitch On");
	imc_log_on=1;
  }
  else
  {
	imc_logstring("hubswitch yes|no");
  }


  return 1;
}

/* save the IMC config file */
int imc_saveconfig(void)
{
  FILE *out;
  imc_info *i;
  char configfile[200];
  char *details;

  if (imc_active == IA_NONE)
    return 0;

  imc_sncpy(configfile, imc_prefix, 193);
  strcat(configfile, "config");

  out = fopen(configfile, "w");
  if (!out)
  {
    imc_lerror("imc_saveconfig: error opening %s", configfile);
    return 0;
  }
  
  fprintf(out,
	  "# Version <config_file_version>\n"
	  "# LocalName <local_imc_name>\n"
	  "# LocalPort <local_imc_port>\n"
	  "# Connection Name:Host:Port:ClientPW:ServerPW:RcvStamp:NoForward:"
	  "Flags\n");

  fprintf(out, "Version 1\n");

  if (imc_active >= IA_CONFIG2)
  {
    fprintf(out, "LocalName %s\n", imc_name);
    fprintf(out, "LocalPort %hu\n", imc_port);
    if (imc_bind!=htonl(INADDR_ANY))
    {
      struct in_addr a;
      a.s_addr=imc_bind;
      fprintf(out, "Bind %s\n", inet_ntoa(a));
    }
  }

  if (imc_siteinfo.name[0])
    fprintf(out, "InfoName %s\n", imc_siteinfo.name);
  if (imc_siteinfo.host[0])
    fprintf(out, "InfoHost %s\n", imc_siteinfo.host);
  if (imc_siteinfo.email[0])
    fprintf(out, "InfoEmail %s\n", imc_siteinfo.email);
  if (imc_siteinfo.imail[0])
    fprintf(out, "InfoImail %s\n", imc_siteinfo.imail);
  if (imc_siteinfo.www[0])
    fprintf(out, "InfoWWW %s\n", imc_siteinfo.www);
  if (imc_siteinfo.details[0])
  {
 	details=imc_siteinfo.details;
	fprintf(out,"InfoDetails ");
	while(*details)
	{
		if(*details == '\r')
		{
			details++;
			continue;
		}
		if(*details == '\n')
		{
			if(details[2])
				 fprintf(out,"\nInfoDetails ");
			else
				fprintf(out,"\n");
		}
		else
		{
			fprintf(out,"%c",*details);
		}
		details++;
	}
   }
  if (imc_siteinfo.flags[0])
    fprintf(out, "InfoFlags %s\n", imc_siteinfo.flags);
  fprintf(out, "MaxGuests %d\n", imc_siteinfo.maxguests);
  if(imc_log_on)
  	fprintf(out, "LogOn yes\n") ;
  else 
  	fprintf(out, "LogOn no\n") ;
  if(imc_hubswitch)
  	fprintf(out, "HubSwitch yes\n") ;
  else 
  	fprintf(out, "HubSwitch no\n") ;
	
  
  for (i=imc_info_list; i; i=i->next)
  {
    if (i->flags & IMC_NOSWITCH) /* if nowswitch set, save entry new or not*/
    	i->flags &= ~IMC_NEW_HUB;
    if (i->flags & IMC_NEW_HUB) /* no giveaway hubs - shogar */
	continue;
    fprintf(out, "Connection %s:%s:%hu:%s:%s:%d:%d:%s\n",
	    i->name,
	    i->host,
	    i->port,
	    i->clientpw,
	    i->serverpw,
	    i->rcvstamp,
	    i->noforward,
	    imc_flagname(i->flags, imc_connection_flags));
   }

  imc_saveconfighook(out);

  if (ferror(out))
  {
    imc_lerror("imc_saveconfig: error saving %s", configfile);
    fclose(out);
    return 0;
  }

  fclose(out);
  return 1;
}

/*  runtime changing of IMC config
 *  returns  >0 success
 *           <0 error
 *          ==0 unknown command
 *
 *  commands:
 *    reload
 *    add <mudname>
 *    delete <mudname>
 *    rename <oldname> <newname>
 *    set <mudname> <host|port|clientpw|serverpw|flags|noforward|
 *                   rcvstamp> <newvalue>
 *    set <mudname> all <host> <port> <clientpw> <serverpw> <noforward>
 *                      <rcvstamp> <flags>
 *    localname <name>
 *    localport <port>
 */

int imc_command(const char *argument)
{
  char arg1[IMC_DATA_LENGTH];
  char arg2[IMC_DATA_LENGTH];
  char arg3[IMC_DATA_LENGTH];
  imc_info *i;

  argument=imc_getarg(argument, arg1, IMC_DATA_LENGTH);
  argument=imc_getarg(argument, arg2, IMC_DATA_LENGTH);

  if (imc_active==IA_NONE)
  {
    imc_qerror("IMC is not initialized");
    return -1;
  }

  if(!strcasecmp(arg1,"log")) /* configurable log messages - shogar */
  {
	imc_log_on = (imc_log_on + 1) % 2;
        if(imc_log_on)
		imc_logstring("Logging On");
        else
		imc_logstring("Logging Off");
        imc_saveconfig();
        return 1;
  }
  if(!strcasecmp(arg1,"hubswitch")) /* configurable log messages - shogar */
  {
	imc_hubswitch = (imc_hubswitch + 1) % 2;
        if(imc_hubswitch)
		imc_logstring("HubSwitch On");
        else
		imc_logstring("HubSwitch Off");
        imc_saveconfig();
        return 1;
  }
  if (!strcasecmp(arg1, "reload"))
  {
    /* reload config file - shut down and restart */
    char *temp;

    if (imc_lock)
    {
      imc_qerror("Reloading the config from within IMC is a Bad Thing <tm>");
      return -1;
    }

    temp=imc_strdup(imc_prefix); /* imc_prefix gets freed, so keep a copy */
    
    imc_shutdown();
    imc_startup(temp);

    imc_strfree(temp);
    return 1;
  }

  if (!arg1[0] || !arg2[0])
    return 0;

  if (!strcasecmp(arg1, "add"))
  {
    if (imc_name && !strcasecmp(arg2, imc_name))
    {
      imc_qerror("%s has been specified as the local mud name. Use 'imc add'"
		 "to add connections to _other_ muds", imc_name);
      return -1;
    }
    
    if (imc_getinfo(arg2)!=NULL)
    {
      imc_qerror("A mud by that name is already configured");
      return -1;
    }

    i=imc_new_info();

    i->name       = imc_strdup(arg2);
    i->host       = imc_strdup("");
    i->clientpw   = imc_strdup("");
    i->serverpw   = imc_strdup("");
	i->connect_attempts = 0;

    return 1;
  }
  else if (!strcasecmp(arg1, "delete"))
  {
    i=imc_getinfo(arg2);

    if (!i)
    {
      imc_qerror("Entry not found");
      return -1;
    }

    imc_delete_info(i);
    imc_saveconfig();

    return 1;
  }
  else if (!strcasecmp(arg1, "rename"))
  {
    i=imc_getinfo(arg2);

    if (!i)
    {
      imc_qerror("Entry not found");
      return -1;
    }

    if (i->connection)
      imc_disconnect(i->name);

    argument=imc_getarg(argument, arg3, IMC_DATA_LENGTH);
    if (!arg3[0])
      return 0;

    imc_strfree(i->name);
    i->name = imc_strdup(arg3);

    imc_saveconfig();
    return 1;
  }
  else if (!strcasecmp(arg1, "set"))
  {
    i=imc_getinfo(arg2);

    if (!i)
    {
      imc_qerror("Entry not found");
      return -1;
    }

    argument=imc_getarg(argument, arg3, IMC_DATA_LENGTH);

    if (!arg3[0] || !argument[0])
      return 0;
    else if (!strcasecmp(arg3, "all"))
    {
      if (i->host)
	imc_strfree(i->host);
      if (i->clientpw)
	imc_strfree(i->clientpw);
      if (i->serverpw)
	imc_strfree(i->serverpw);

      argument=imc_getarg(argument, arg3, IMC_DATA_LENGTH);
      i->host=imc_strdup(arg3);
      argument=imc_getarg(argument, arg3, IMC_DATA_LENGTH);
      i->port=strtoul(arg3, NULL, 10);
      argument=imc_getarg(argument, arg3, IMC_PW_LENGTH);
      i->clientpw=imc_strdup(arg3);
      argument=imc_getarg(argument, arg3, IMC_PW_LENGTH);
      i->serverpw=imc_strdup(arg3);
      argument=imc_getarg(argument, arg3, IMC_DATA_LENGTH);
      i->rcvstamp=strtoul(arg3, NULL, 10);
      argument=imc_getarg(argument, arg3, IMC_DATA_LENGTH);
      i->noforward=strtoul(arg3, NULL, 10);
      argument=imc_getarg(argument, arg3, IMC_DATA_LENGTH);
      i->flags=imc_flagvalue(arg3, imc_connection_flags);

      imc_saveconfig();

      return 1;
    }
    else if (!strcasecmp(arg3, "host"))
    {
      if (i->host)
	imc_strfree(i->host);
      i->host=imc_strdup(argument);

      imc_saveconfig();
      return 1;
    }
    else if (!strcasecmp(arg3, "port"))
    {
      i->port=strtoul(argument, NULL, 10);

      imc_saveconfig();
      return 1;
    }
    else if (!strcasecmp(arg3, "clientpw"))
    {
      if (i->clientpw)
	imc_strfree(i->clientpw);
      i->clientpw=imc_strdup(argument);

      imc_saveconfig();
      return 1;
    }
    else if (!strcasecmp(arg3, "serverpw"))
    {
      if (i->serverpw)
	imc_strfree(i->serverpw);
      i->serverpw=imc_strdup(argument);

      imc_saveconfig();
      return 1;
    }
    else if (!strcasecmp(arg3, "rcvstamp"))
    {
      i->rcvstamp=strtoul(argument, NULL, 10);

      imc_saveconfig();
      return 1;
    }
    else if (!strcasecmp(arg3, "noforward"))
    {
      i->noforward=strtoul(argument, NULL, 10);

      imc_saveconfig();
      return 1;
    }
    else if (!strcasecmp(arg3, "flags"))
    {
      i->flags ^=imc_flagvalue(argument, imc_connection_flags);

      imc_saveconfig();
      return 1;
    }

    return 0;
  }
  else if (!strcasecmp(arg1, "localname"))
  {
    if (imc_lock)
    {
      imc_qerror("Changing localname from within IMC is a Bad Thing <tm>");
      return -1;
    }

    /* shut down IMC, change name, and restart */
    if (imc_active >= IA_UP)
      imc_shutdown_network();

    if (imc_name)
      imc_strfree(imc_name);

    imc_name=imc_strdup(arg2);
    imc_active=IA_CONFIG2;

    imc_startup_network();
    imc_saveconfig();

    return 1;
  }
  else if (!strcasecmp(arg1, "localport"))
  {
    int p=atoi(arg2);

    if ((p<1024 || p>65535) && p!=0)
    {
      imc_qerror("Port number must be 0 or 1024..65535");
      return -1;
    }

    if (imc_active >= IA_LISTENING)
      imc_shutdown_port();

    imc_port=p;

    if (imc_active == IA_UP && imc_port)
      imc_startup_port();

    imc_saveconfig();

    return 1;
  }
  else if (!strcasecmp(arg1, "info"))
  {
    if (!strcasecmp(arg2, "name"))
    {
      imc_strfree(imc_siteinfo.name);
      imc_siteinfo.name=imc_strdup(argument);

      if (imc_active == IA_CONFIG2)
	imc_startup_network();
    }
    else if (!strcasecmp(arg2, "host"))
    {
      imc_strfree(imc_siteinfo.host);
      imc_siteinfo.host=imc_strdup(argument);
    }
    else if (!strcasecmp(arg2, "email"))
    {
      imc_strfree(imc_siteinfo.email);
      imc_siteinfo.email=imc_strdup(argument);
      
      if (imc_active == IA_CONFIG2)
	imc_startup_network();
    }
    else if (!strcasecmp(arg2, "imail"))
    {
      imc_strfree(imc_siteinfo.imail);
      imc_siteinfo.imail=imc_strdup(argument);
    }
    else if (!strcasecmp(arg2, "www"))
    {
      imc_strfree(imc_siteinfo.www);
      imc_siteinfo.www=imc_strdup(argument);
    }
    else if (!strcasecmp(arg2, "details"))
    {
      imc_strfree(imc_siteinfo.details);
      imc_siteinfo.details=imc_strdup(argument);
    }
    else if (!strcasecmp(arg2, "flags"))
    {
      imc_strfree(imc_siteinfo.flags);
      imc_siteinfo.flags=imc_strdup(argument);
    }
    else
      return 0;

    imc_saveconfig();
    return 1;
  }

  return 0;
}

/* read an IMC rignores file */
int imc_readignores(void)
{
  FILE *inf;
  char buf[1000];
  char name[200];
  char arg[IMC_NAME_LENGTH];
  int type;
  int count;

  imc_sncpy(name, imc_prefix, 191);
  strcat(name, "rignores");

  inf=fopen(name, "r");
  if (!inf)
  {
    imc_logerror("imc_readignores: couldn't open %s", name);
    return 0;
  }

  while (!ferror(inf) && !feof(inf))
  {
    if (fgets(buf, 1000, inf) == NULL)
      break;

    if (buf[0] == '#' || buf[0] == '\n')
      continue;

    sscanf(buf, "%[^ \n]%n", arg, &count);
    type=imc_statevalue(buf+count, imc_ignore_types);
    imc_addignore(arg, type);  /* add the entry */
  }

  if (ferror(inf))
  {
    imc_lerror("imc_readignores");
    fclose(inf);
    return 0;
  }

  fclose(inf);
  return 1;
}

/* save the current rignore list */
int imc_saveignores(void)
{
  FILE *out;
  char name[200];
  imc_ignore_data *ign;
  
  imc_sncpy(name, imc_prefix, 191);
  strcat(name, "rignores");

  out = fopen(name, "w");
  if (!out)
  {
    imc_lerror("imc_saveignores: error opening %s", name);
    return 0;
  }

  fprintf(out,
	  "# IMC rignores file, one name per line, no leading spaces\n"
	  "# types: ignore, notrust, trust\n"
	  "# lines starting with '#' are discarded\n");

  for (ign=imc_ignore_list; ign; ign=ign->next)
    fprintf(out, "%s%s%s %s\n",
	    (ign->match & IMC_IGNORE_PREFIX) ? "*" : "",
	    ign->name,
	    (ign->match & IMC_IGNORE_SUFFIX) ? "*" : "",
	    imc_statename(ign->type, imc_ignore_types));

  if (ferror(out))
  {
    imc_lerror("imc_saveignores: error saving %s", name);
    fclose(out);
    return 0;
  }

  fclose(out);
  return 1;
}

imc_ignore_data *imc_newignore(void)
{
  imc_ignore_data *ign;

  ign=imc_malloc(sizeof(*ign));
  ign->name=NULL;
  ign->match=IMC_IGNORE_EXACT;
  ign->type=-1;
  ign->next=imc_ignore_list;
  imc_ignore_list=ign;

  return ign;
}

void imc_freeignore(imc_ignore_data *ign)
{
  if (ign->name)
    imc_strfree(ign->name);

  imc_free(ign, sizeof(*ign));
}

int imc_delignore(const char *what)
{
  imc_ignore_data *ign, *last;
  char *who;
  char buf[IMC_DATA_LENGTH];
  int match=0;

  strcpy(buf, what);
  who=buf;

  if (who[0]=='*')
  {
    who++;
    match|=IMC_IGNORE_PREFIX;
  }

  if (who[0] && who[strlen(who)-1]=='*')
  {
    who[strlen(who)-1]=0;
    match|=IMC_IGNORE_SUFFIX;
  }

  for (last=NULL, ign=imc_ignore_list; ign; last=ign, ign=ign->next)
    if (match==ign->match && !strcasecmp(who, ign->name))
    {
      if (!last)
	imc_ignore_list=ign->next;
      else
	last->next=ign->next;

      imc_freeignore(ign);
      return 1;
    }
  
  return 0;
}

void imc_addignore(const char *what, int type)
{
  imc_ignore_data *ign;
  char buf[IMC_DATA_LENGTH];
  char *who;
  int match=0;

  ign=imc_newignore();
  ign->type=type;

  strcpy(buf, what);
  who=buf;

  if (who[0]=='*')
  {
    match |= IMC_IGNORE_PREFIX;
    who++;
  }
  
  if (who[0] && who[strlen(who)-1]=='*')
  {
    who[strlen(who)-1]=0;
    match |= IMC_IGNORE_SUFFIX;
  }

  ign->match=match;
  ign->name=imc_strdup(who);
}

/* add/remove/list rignores */
const char *imc_ignore(const char *what)
{
  int count;
  imc_ignore_data *ign;
  char arg[IMC_NAME_LENGTH];
  int type;

  what=imc_getarg(what, arg, IMC_NAME_LENGTH);

  if (!arg[0])
  {
    char *buf=imc_getsbuf(IMC_DATA_LENGTH);
    strcpy(buf, "Current entries:\n\r");
    for (count=0, ign=imc_ignore_list; ign; ign=ign->next, count++)
      sprintf(buf + strlen(buf), " %10s   %s%s%s\n\r",
	      imc_statename(ign->type, imc_ignore_types),
	      (ign->match & IMC_IGNORE_PREFIX) ? "*" : "",
	      ign->name,
	      (ign->match & IMC_IGNORE_SUFFIX) ? "*" : "");

    if (!count)
      strcat(buf, " none");
    else
      sprintf(buf + strlen(buf), "[total %d]", count);

    imc_shrinksbuf(buf);
    return buf;
  }

  if (!what[0])
    return "Must specify both action and name.";

  if (!strcasecmp(arg, "delete"))
  {
    if (imc_delignore(what))
    {
      imc_saveignores();
      return "Entry deleted.";
    }
    return "Entry not found.";
  }

  type=imc_statevalue(arg, imc_ignore_types);
  if (type<0)
    return "Unknown ignore type";

  imc_addignore(what, type);
  imc_saveignores();
  
  return "Entry added.";
}

/* check if needle is in haystack (case-insensitive) */
static int substr(const char *needle, const char *haystack)
{
  int len=strlen(needle);

  if (!len)
    return 1;

  while ((haystack=strchr(haystack, *needle))!=NULL)
    if (!strncasecmp(haystack, needle, len))
      return 1;

  return 0;
}

/* find ignore data on someone */
imc_ignore_data *imc_findignore(const char *who, int type)
{
  imc_ignore_data *ign;
  int len;
  int wlen=strlen(who);

  for (ign=imc_ignore_list; ign; ign=ign->next)
  {
    if (type>=0 && type!=ign->type)
      continue;
    
    len=strlen(ign->name);

    switch (ign->match)
    {
    case 0: /* exact match */
      if (!strcasecmp(ign->name, who))
	return ign;
      break;
    case IMC_IGNORE_SUFFIX: /* abcd* */
      if (!strncasecmp(ign->name, who, len))
	return ign;
      break;
    case IMC_IGNORE_PREFIX: /* *abcd */
      wlen=strlen(who);
      if (wlen>=len && !strncasecmp(ign->name, who+wlen-len, len))
	return ign;
      break;
    case IMC_IGNORE_PREFIX|IMC_IGNORE_SUFFIX: /* *abcd* */
      if (substr(ign->name, who))
        return ign;
      break;
    }
  }

  return NULL;
}

/* check if a packet from a given source should be ignored */
int imc_isignored(const char *who)
{
  return (imc_findignore(who, IMC_IGNORE)!=NULL);
}


/* show current IMC socket states */
const char *imc_sockets(void)
{
  imc_connect *c;
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  char *state;
  int r, s;

  if (imc_active<IA_UP)
    return "IMC is not active.\n\r";

  sprintf(buf, "%4s %-9s %-15s %-11s %-11s %-3s %-3s\n\r",
	  "Desc", "Mud", "State", "Inbuf", "Outbuf", "Sp1", "Sp2");

  for (c=imc_connect_list; c; c=c->next)
  {
    switch (c->state)
    {
    case IMC_CLOSED:
      state = "closed";
      break;
    case IMC_CONNECTING:
      state = "connecting";
      break;
    case IMC_WAIT1:
      state = "wait1";
      break;
    case IMC_WAIT2:
      state = "wait2";
      break;
    case IMC_CONNECTED:
      state = "connected";
      break;
    default:
      state = "unknown";
      break;
    }
    
#ifdef USEIOCTL
    /* try to work out the system buffer sizes */
    r=0;
    ioctl(c->desc, TIOCINQ, &r);
    r += strlen(c->inbuf);
    
    s=outqsize;
    if (s)
    {
      ioctl(c->desc, TIOCOUTQ, &s);
      s=outqsize-s;
    }
    s += strlen(c->outbuf);
#else
    r=strlen(c->inbuf);
    s=strlen(c->outbuf);
#endif

    sprintf(buf+strlen(buf),
	    "%4d %-9s %-15s %5d/%-5d %5d/%-5d %3d %3d\n\r",
	    c->desc,
	    c->info ? c->info->name : "unknown",
	    state,
	    r,
	    c->insize,
	    s,
	    c->outsize,
	    c->spamcounter1,
	    c->spamcounter2);
  }

  imc_shrinksbuf(buf);
  return buf;
}

/*  list current connections/known muds
 *  level=0 is mortal-level access (mudnames and connection states)
 *  level=1 is imm-level access (names, hosts, ports, states)
 *  level=2 is full access (names, hosts, ports, passwords, flags, states)
 *  level=3 is all known muds on IMC, and no direct connection status
 *  level=4 is IMC local config info (mortal-safe)
 *  level=5 is IMC local config info
 */
const char *imc_list(int level)
{
  imc_info *i;
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  char *state;
  imc_reminfo *p;

  if (level<=2)
  {
    strcpy(buf, "Direct connections:\n\r");
    
    switch (level)
    {
    case 0:
      sprintf(buf + strlen(buf), "%-10s %-15s %-8s", "Name", "State", "LastConn");
      break;
    case 1:
      sprintf(buf + strlen(buf), "%-10s %-30s %5s %-13s %-8s", "Name", "Host",
	      "Port", "State", "LastConn");
      break;
    case 2:
                 /* show failed reconnect attempts - shogar */
      sprintf(buf + strlen(buf),
	      "%-8s %-25s %5s %-13s %-10s %-10s\n"
	      "         %-8s %-9s %-5s %-5s %-8s %-8s %s",
	      "Name", "Host", "Port", "State", "ClientPW", "ServerPW",
	      "RcvStamp", "NoForward", "Timer", "TimeD", "LastConn", "Attempts","Flags");
      break;
    }
    
    for (i=imc_info_list; i; i=i->next)
    {
      char lastconn[20];
      
      state = i->connection ? "connected" : "not connected";

      if (i->connection)
        lastconn[0]=0;
      else if (!i->last_connected)
	strcpy(lastconn, "never");
      else
      {
	long diff=(long)imc_now - (long)i->last_connected;
	
	sprintf(lastconn, "%5ld:%02ld", diff/3600, (diff/60) % 60);
      }

      switch (level)
      {
      case 0:
	sprintf(buf + strlen(buf), "\n\r%-10s %-15s %8s", i->name, state, lastconn);
	break;
      case 1:
	sprintf(buf + strlen(buf), "\n\r%-10s %-30s %5hu %-13s %8s",
		i->name,
		i->host,
		i->port,
		state,
		lastconn);
	break;
      case 2:
	sprintf(buf + strlen(buf),
		"\n\r%-8s %-25s %5hu %-13s %-10s %-10s"
		"\n\r         %-8d %-9d %-5d %-5ld %8s %-8d %s",
		i->name,
		i->host,
		i->port,
		state,
		i->clientpw,
		i->serverpw,
		i->rcvstamp,
		i->noforward,
		imc_next_event(ev_reconnect, i),
		i->timer_duration,
		lastconn,
                i->connect_attempts, /* show failed reconnects - shogar */
		imc_flagname(i->flags, imc_connection_flags));
	break;
      }
    }
    
    imc_shrinksbuf(buf);
    return buf;
  }

  if (level==3 || level==6)
  {
    strcpy(buf, "Active muds on IMC:\n\r");
    if (imc_is_router)
      sprintf(buf + strlen(buf), "%-10s  %-10s  %-9s  %-20s  %-10s",
	      "Name", "Last alive", "Ping time", "IMC Version", "Route");
    else
      sprintf(buf + strlen(buf), "%-10s  %-10s  %-20s  %-10s",
	      "Name", "Last alive", "IMC Version", "Route");
    
    for (p=imc_reminfo_list; p; p=p->next)
    {
      if (p->hide && level==3)
	continue;
      
      if (imc_is_router)
      {
	if (p->ping)
        {
	  sprintf(buf + strlen(buf), "\n\r%-10s  %9ds  %7dms  %-20s  %-10s %s",
		  p->name, (int) (imc_now - p->alive), p->ping, p->version,
		  p->route ? p->route : "broadcast", p->type ? "expired" : "");
        }
	else
        {
	  sprintf(buf + strlen(buf), "\n\r%-10s  %9ds  %9s  %-20s  %-10s %s",
		  p->name, (int) (imc_now - p->alive), "unknown", p->version,
		  p->route ? p->route : "broadcast", p->type ? "expired" : "");
        }
      }
      else
      {
	sprintf(buf + strlen(buf), "\n\r%-10s  %9ds  %-20s  %-10s %s",
		p->name, (int) (imc_now - p->alive), p->version,
		p->route ? p->route : "broadcast", p->type ? "expired" : "");
      }
    }

    imc_shrinksbuf(buf);
    return buf;
  }

  if (level==4)
  {
    sprintf(buf,
	    "Local IMC configuration:\n\r"
	    "  IMC name:    %s\n\r"
	    "  IMC version: %s\n\r"
	    "  IMC state:   %s",
	    imc_name ? imc_name : "not set",
	    IMC_VERSIONID,
	    (imc_active>=IA_UP) ? "active" : "not active");
    imc_shrinksbuf(buf);
    return buf;
  }

  if (level==5)
  {
    sprintf(buf,
	    "Local IMC configuration:\n\r"
	    "  IMC name:    %s\n\r"
	    "  IMC port:    %hu\n\r"
	    "  IMC version: %s\n\r"
	    "  IMC state:   %s\n\r"
	    "Site information:\n\r"
	    "  Full name:   %s\n\r"
	    "  Host/port:   %s\n\r"
	    "  Email:       %s\n\r"
	    "  IMC mail:    %s\n\r"
	    "  Webpage:     %s\n\r"
	    "  Details:     %s\n\r"
	    "  Flags:       %s\n\r",
	    
	    (imc_active>=IA_CONFIG2) ? imc_name : "not set",
	    imc_port,
	    IMC_VERSIONID,
	    imc_statename(imc_active, imc_active_names),

	    imc_siteinfo.name,
	    imc_siteinfo.host,
	    imc_siteinfo.email,
	    imc_siteinfo.imail,
	    imc_siteinfo.www,
	    imc_siteinfo.details,
	    imc_siteinfo.flags);
    
    imc_shrinksbuf(buf);
    return buf;
  }

  imc_shrinksbuf(buf);
  return "Bad invocaton of imc_list.";
}

/* get some IMC stats, return a string describing them */
const char *imc_getstats(void)
{
  char *buf=imc_getsbuf(300);
  int evcount;
  imc_event *ev;

  for (evcount=0, ev=imc_event_list; ev; ev=ev->next, evcount++)
    ;

  sprintf(buf,
	  "IMC statistics\n\r"
	  "\n\r"
	  "Received packets:    %ld\n\r"
	  "Received bytes:      %ld (%ld/second)\n\r"
	  "Transmitted packets: %ld\n\r"
	  "Transmitted bytes:   %ld (%ld/second)\n\r"
	  "Maximum packet size: %d\n\r"
	  "Pending events:      %d\n\r"
#ifdef IDEBUG
	  "Sequence drops:      %d\n\r"
          /* added some additional info here - shogar */
          "Time between keepalives recieved:  %ld seconds\n\r"
          "Time between pings:  %ld seconds\n\r" 
          "Time between keepalives sent:  %ld seconds\n\r",
#else
	  "Sequence drops:      %d\n\r"
          "Logging %s\n\r"
          "HubSwitch %s\n\r",
#endif

	  imc_stats.rx_pkts,
	  imc_stats.rx_bytes,
	  imc_stats.rx_bytes /
	  ((imc_now - imc_stats.start) ?
	   (imc_now - imc_stats.start) : 1),
	  imc_stats.tx_pkts,
	  imc_stats.tx_bytes,
	  imc_stats.tx_bytes /
	  ((imc_now - imc_stats.start) ?
	   (imc_now - imc_stats.start) : 1),
	  imc_stats.max_pkt,
	  evcount,
#ifdef IDEBUG
          /* added some additional info here - shogar */
	  imc_stats.sequence_drops,
          time_since_last_ralive,
          time_since_last_ping,
          time_since_last_salive); 
#else
	  imc_stats.sequence_drops,
          imc_log_on ? " yes" : " no",
          imc_hubswitch ? " yes" : " no" );
#endif
   sprintf(buf+strlen(buf),"%s\n\r",REVISION);
   sprintf(buf+strlen(buf),"Last IMC Boot: %s\n\r",ctime(&imc_boot));

  imc_shrinksbuf(buf);
  return buf;
}


