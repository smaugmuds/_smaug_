##[_--{SMAUG}--_] (http://smaugmuds.github.io/_smaug_)

###Introduction

```
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
    |                             README                                  |
    |_____________________________________________________________________|
    |                                                                     |
    |                            --(src)--                                |
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

```

**--{ [S]imulated [M]edieval [A]dventure Multi-[U]ser [G]ame }--**

**--{SMAUG 2.0}--** is an enhancenment of **SMAUG 1.8b**, this project was started to have an stable source code structure, which will build, install and run in most modern distributed system environments. **--{SMAUG 2.0}--** is ready to add, enable and disable new code snippets using the new **GNU Autotools** build system. Many bug fixes and additions were made to the **SMAUG 1.8b** source code, always trying to keep the compatibility with **SMAUG** previous revisions. All path definitions for data files and directories were modified, **--{SMAUG}--** installation is now stored into a modern system directory distribution. Old "**startd**" script was replaced with "**smaugd**" and "**smaugd-monitor**", these scripts will start up and keep your MUD server running. If the **--{SMAUG}--** server process fails, "**smaugd-monitor**" will respawn it again. To know more about all these new changes, take a look at the section **--{SMAUG}-- ChangeLog**.

**SMAUG 1.8b**, is an fixed **SMAUG 1.4a** which is a heavily modified derivative of the **Merc 2.1** distribution. **SMAUG 1.4a** is close to six times the size of **Merc 2.1**, and has major changes to the inner workings of the code, hence making porting of code from other Diku-derivatives non-trivial to the non-coder. Due to the ability to edit most everything online, you will most likely find the online help pages more informative. **SMAUG** is the base code that runs the popular **Realms of Despair** MUD. Through the years it has been tweaked, modified, and enhanced to become one of the most feature packed branches of the **MERC** family of codebases. 

**SMAUG** is a derivative of **Merc Diku Mud** is a **Diku Mud** with many enhancements and contributions. It was written in 1990 and 1991 by **Sebastian Hammer**, **Tom Madsen**, **Katja Nyboe**, **Michael Seifert**, and **Hans Henrik StÃŠrfeldt** at **DIKU** _(Datalogisk Institut KÃžbenhavns Universitet) at the department of computer science at the University of Copenhagen in Copenhagen, Denmark_. Commonly referred to as simply **"Diku"**, the game was greatly inspired by **AberMUD**, though **Diku** became one of the first multi-usergames to become popular as a freely-available program for its gameplay and similarity to **Dungeons & Dragons**.

**AberMUD** was the first popular open source MUD. It was named after the town _Aberystwyth_, in which it was written. The first version was written in **B** by **Alan Cox**, **Richard Acott**, **Jim Finnis**,and **Leon Thrane** based at **University of Wales**, _Aberystwyth_ for an old **Honeywell** mainframe and opened in 1987.

 
### Authors and Contributors

**--{SMAUG 2.0}--** by **Antonio Cao** ([@burzumishi] (https://github.com/burzumishi))
Wed Jun 11 14:49:27 CEST 2014

Feel free to contribute with **--{SMAUG}-- in GitHub**.


### Copyright and License

**Diku Mud** is copyright (C) 1990, 1991 by Sebastian Hammer, Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.  Their license agreement is in the file **'license.doc'**.

**Merc Diku Mud** is a derivative of the original **Diku Mud** and is subject to their copyright and license agreement.  **Merc Diku Mud** contains substantial enhancements to **Diku Mud**.  These enhancements are copyright 1992, 1993 by Michael Chastain, Michael Quan, and Mitchell Tse.  Our license agreement is in **'license.txt'**.

The license terms boil down to this: the people who wrote this mud want due credit for their work.

**MERC Industries** had officially disolved October 13, 1993.  **MERC 2.2** is a compilation of various bug and typo fixes along with a few new worlds.  **MERC 2.2** also has a variable line pager, MOBPrograms, and player settable prompt.

**SMAUG** is a derivative of the version **2.1** distribution of **Merc Diku Mud**. **SMAUG** contains substantial enhancements to **Merc Diku Mud**, including some features of the original **Diku Mud** that were removed by the Merc team.

**--{SMAUG}--** is a derivative of the original **SMAUG** and is subject to their copyright and license agreement. **--{SMAUG}--** contains substantial enhancements to **SMAUG**.  These enhancements are copyright 2014-2015 by **Antonio Cao** ([@burzumishi] (https://github.com/burzumishi)) under the **'GNU GPL v2 LICENSE'**.

**Thanks to the SMAUG Code Team!**

==============

_**--{SMAUG}--** (c) 2014-2015 Antonio Cao (@burzumishi)_
