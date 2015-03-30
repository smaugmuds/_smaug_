/*
 * source file for mxp
 *
 * Originally by Brian Graversen
 * Modified for MERC and Support for all MXP Tags by Celestian (celestian1@gmail.com)
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/telnet.h>

#include "mud.h"

/*
 * mxp_to_char()
 * -------------
 *
 * When sending MXP messages with this function, you should
 * always use <BR> instead of \n\r, since \n\r resets the
 * state of MXP. <BR> will do a linebreak just like \n\r,
 * but the state is kept.
 *
 * The following MXP tags are supported, and works well
 * in zmud 6.16 and mushclient 3.17. Other clients may
 * work, but it's very unlikely.
 *
 * <B>text</B>   : BOLD
 * <I>text</I>   : ITALIC
 * <U>text</U>   : UNDERLINE
 * <BR>          : LINEBREAK (is replaced with \n\r if no MXP)
 *
 * Some of these more advanced features are also supported
 * -------------------------------------------------------
 * <A href="URL">text</A>    : web hyperlinks.
 * <SEND [args]>text</SEND>  : mud hyperlinks and menus.
 * <EXPIRE>                  : used for stopping links.
 *
 */
void mxp_to_char(char *txt, CHAR_DATA *ch, int mxp_style)
{
  char buf[2*MAX_STRING_LENGTH];
  int i = 0, j = 0;

  if (!ch->desc)
    return;

  if (USE_MXP(ch))
  {
    switch(mxp_style)
    {
      default:
	bug("Mxp_to_char: strange style '%d'.", mxp_style);
	strcpy(buf, txt);
      case MXP_SAFE:
        sprintf(buf, "%s%s%s", "\e[0z", txt, "\e[7z");
	break;
      case MXP_ALL:
	/* There is a bug in zmud 6.16 so we can not do the obvious
	 * here. Instead we'll use this workaround. In the future,
	 * the bug in zmud might be fixed, and the following line
	 * can replace everything else.
	 *
	 *   sprintf(buf, "%s%s%s", "\e[1z", txt, "\e[7z");
	 *
	 */
	buf[j++] = '\e'; buf[j++] = '['; buf[j++] = '1'; buf[j++] = 'z';
	while (txt[i] != '\0')
	{
          switch(txt[i])
	  {
            default:
	      buf[j++] = txt[i++];
	      break;
            case '<':
	      if (!memcmp(&txt[i], "<BR>", strlen("<BR>")))
	      {
	        i += strlen("<BR>");
		buf[j++] = '\n'; buf[j++] = '\r';
                buf[j++] = '\e'; buf[j++] = '['; buf[j++] = '1'; buf[j++] = 'z';
	      }
              else
                buf[j++] = txt[i++];
	      break;
	  }
	}
        buf[j++] = '\e'; buf[j++] = '['; buf[j++] = '7'; buf[j++] = 'z';
        buf[j] = '\0';
    	break;
      case MXP_NONE:
        sprintf(buf, "%s%s", "\e[7z", txt);
	break;
    }
  }
  else
  {
    while(txt[i] != '\0')
    {
      switch(txt[i])
      {
        default:
	  buf[j++] = txt[i++];
	  break;
        case '<':
	  if (!memcmp(&txt[i], "<B>", strlen("<B>")))
	    i += strlen("<B>");
	  else if (!memcmp(&txt[i], "</B>", strlen("</B>")))
	    i += strlen("</B>");
	  else if (!memcmp(&txt[i], "<U>", strlen("<U>")))
	    i += strlen("<U>");
	  else if (!memcmp(&txt[i], "</U>", strlen("</U>")))
	    i += strlen("</U>");
	  else if (!memcmp(&txt[i], "<I>", strlen("<I>")))
	    i += strlen("<I>");
	  else if (!memcmp(&txt[i], "</I>", strlen("</I>")))
	    i += strlen("</I>");
	  else if (!memcmp(&txt[i], "<H>", strlen("<H>")))
	    i += strlen("</I>");
	  else if (!memcmp(&txt[i], "</H>", strlen("</H>")))
	    i += strlen("</I>");
	  else if (!memcmp(&txt[i], "<S>", strlen("<S>")))
	    i += strlen("</I>");
	  else if (!memcmp(&txt[i], "</S>", strlen("</S>")))
	    i += strlen("</I>");
	  else if (!memcmp(&txt[i], "<BR>", strlen("<BR>")))
	  {
            if (mxp_style == MXP_ALL)
            {
              buf[j++] = '\n';
              buf[j++] = '\r';
            }
	    i += strlen("<BR>");
	  }
	  else if (!memcmp(&txt[i], "<SEND", strlen("<SEND")))
	  {
	    i += strlen("<SEND");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<A", strlen("<A")))
	  {
	    i += strlen("<A");
	    while(txt[i++] != '>')
              ;
	  }
          else if (!memcmp(&txt[i], "<EXPIRE", strlen("<EXPIRE")))
          {
            i += strlen("<EXPIRE");
            while(txt[i++] != '>')
              ;
          }
	  else if (!memcmp(&txt[i], "<C", strlen("<C")))
	  {
	    i += strlen("<C");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<COLOR", strlen("<COLOR")))
	  {
	    i += strlen("<COLOR");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<FONT", strlen("<FONT")))
	  {
	    i += strlen("<FONT");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<IMAGE", strlen("<IMAGE")))
	  {
	    i += strlen("<IMAGE");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<!ELEMENT", strlen("<!ELEMENT")))
	  {
	    i += strlen("<!ELEMENT");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<!EL", strlen("<!EL")))
	  {
	    i += strlen("<!EL");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<!ATTLIST", strlen("<!ATTLIST")))
	  {
	    i += strlen("<!ATTLIST");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<!AT", strlen("<!AT")))
	  {
	    i += strlen("<!AT");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<!ENTITY", strlen("<!ENTITY")))
	  {
	    i += strlen("<!ENTITY");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<!EN", strlen("<!EN")))
	  {
	    i += strlen("<!EN");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<VAR", strlen("<VAR")))
	  {
	    i += strlen("<VAR");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<V", strlen("<V")))
	  {
	    i += strlen("<V");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<ESC", strlen("<ESC")))
	  {
	    i += strlen("<ESC");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<!TAG", strlen("<!TAG")))
	  {
	    i += strlen("<!TAG");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<NOBR", strlen("<NOBR")))
	  {
	    i += strlen("<NOBR");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<P", strlen("<P")))
	  {
	    i += strlen("<P");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<SBR", strlen("<SBR")))
	  {
	    i += strlen("<SBR");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<VERSION", strlen("<VERSION")))
	  {
	    i += strlen("<VERSION");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<SUPPORT", strlen("<SUPPORT")))
	  {
	    i += strlen("<SUPPORT");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<H1", strlen("H1")))
	  {
	    i += strlen("<H1");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<H2", strlen("<H2")))
	  {
	    i += strlen("<H2");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<H3", strlen("<H3")))
	  {
	    i += strlen("<H3");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<H4", strlen("<H4")))
	  {
	    i += strlen("<H4");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<H5", strlen("<H5")))
	  {
	    i += strlen("<H5");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<H6", strlen("<H6")))
	  {
	    i += strlen("<H6");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<HR", strlen("<HR")))
	  {
	    i += strlen("<HR");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<SMALL", strlen("<SMALL")))
	  {
	    i += strlen("<SMALL");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<TT", strlen("<TT")))
	  {
	    i += strlen("<TT");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<GAUGE", strlen("<GAUGE")))
	  {
	    i += strlen("<GAUGE");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<STAT", strlen("<STAT")))
	  {
	    i += strlen("<STAT");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<FRAME", strlen("<FRAME")))
	  {
	    i += strlen("<FRAME");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<FRAME", strlen("<FRAME")))
	  {
	    i += strlen("<FRAME");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<DEST", strlen("<DEST")))
	  {
	    i += strlen("<DEST");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<RELOCATE", strlen("<RELOCATE")))
	  {
	    i += strlen("<RELOCATE");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "<SOUND", strlen("<SOUND")))
	  {
	    i += strlen("<SOUND");
	    while(txt[i++] != '>')
              ;
	  }
	  else if (!memcmp(&txt[i], "</HR>", strlen("</HR>")))
	    i += strlen("</H1>");
	  else if (!memcmp(&txt[i], "</H2>", strlen("</H2>")))
	    i += strlen("</H2>");
	  else if (!memcmp(&txt[i], "</H3>", strlen("</H3>")))
	    i += strlen("</H3>");
	  else if (!memcmp(&txt[i], "</H4>", strlen("</H4>")))
	    i += strlen("</H4>");
	  else if (!memcmp(&txt[i], "</H5>", strlen("</H5>")))
	    i += strlen("</H5>");
	  else if (!memcmp(&txt[i], "</H5>", strlen("</H5>")))
	    i += strlen("</H5>");
	  else if (!memcmp(&txt[i], "</H1>", strlen("</H1>")))
	    i += strlen("</H1>");
	  else if (!memcmp(&txt[i], "</SMALL>", strlen("</SMALL>")))
	    i += strlen("</SMALL>");
	  else if (!memcmp(&txt[i], "</TT>", strlen("</TT>")))
	    i += strlen("</TT>");
	  else if (!memcmp(&txt[i], "&nbsp;", strlen("&nbsp;")))
	    i += strlen("&nbsp;");
	  else if (!memcmp(&txt[i], "</SEND>", strlen("</SEND>")))
	    i += strlen("</SEND>");
	  else if (!memcmp(&txt[i], "</A>", strlen("</A>")))
	    i += strlen("</A>");
	  else if (!memcmp(&txt[i], "</C>", strlen("</C>")))
	    i += strlen("</C>");
	  else if (!memcmp(&txt[i], "</COLOR>", strlen("</COLOR>")))
	    i += strlen("</COLOR>");
	  else if (!memcmp(&txt[i], "</FONT>", strlen("</FONT>")))
	    i += strlen("</FONT>");
          else
	    buf[j++] = txt[i++];
	  break;
      }
    }
    buf[j] = '\0';
  }

  send_to_char(buf, ch);
}

void init_mxp(DESCRIPTOR_DATA *d)
{
  const char enable_mxp[] = { IAC, SB, TELOPT_MXP, IAC, SE, '\0' };

  write_to_buffer(d, enable_mxp, 0);
  write_to_buffer(d, "\e[7z", 0); /* default to locked mode */
  d->mxp = TRUE;
}

void shutdown_mxp(DESCRIPTOR_DATA *d)
{
  d->mxp = FALSE;
}

void do_mxp(CHAR_DATA *ch, char *argument)
{
  if (!ch->desc)
    return;

  if (!ch->desc->mxp)
  {
    send_to_char("Sorry, your client does not support MXP.\n\r", ch);
  if (xIS_SET(ch->act, PLR_MXP))
  {
    xREMOVE_BIT(ch->act, PLR_MXP);
    send_to_char("MXP has been disabled.\n\r", ch);
  }
    return;
  }
  if (xIS_SET(ch->act, PLR_MXP))
  {
    xREMOVE_BIT(ch->act, PLR_MXP);
    send_to_char("MXP has been disabled.\n\r", ch);
  }
  else
  {
    xSET_BIT(ch->act, PLR_MXP);
    send_to_char("MXP has been enabled.\n\r", ch);
  }
}
