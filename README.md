##[_{SMAUG}_] (http://smaugmuds.github.io/_smaug_)

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
    |                              README                                 |
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

**--{SMAUG}--** is a multiplayer text-based role-playing game, which is a type of MUD. It can be used and modified to create your own game server.

**--{SMAUG}--** is an enhancenment of **AFKMud 2.1.5**, the goal is to develop a new structure to build and run in modern distributed system environments. **AFKMud**, is an fixed **SMAUG 1.4a** which is a heavily modified derivative of the **Merc 2.1** distribution. Stock area zones from **SMAUG 1.4a** have been restored into **--{smaug}--**.

**SMAUG 1.4a** is close to six times the size of **Merc 2.1**, and has major changes to the inner workings of the code, hence making porting of code from other Diku-derivatives non-trivial to the non-coder. Due to the ability to edit most everything online, you will most likely find the online help pages more informative. **SMAUG** is the base code that runs the popular **Realms of Despair** MUD. Through the years it has been tweaked, modified, and enhanced to become one of the most feature packed branches of the **MERC** family of codebases. 

**SMAUG** is a derivative of **Merc Diku Mud** is a **Diku Mud** with many enhancements and contributions. It was written in 1990 and 1991 by **Sebastian Hammer**, **Tom Madsen**, **Katja Nyboe**, **Michael Seifert**, and **Hans Henrik StÃ¦rfeldt** at **DIKU** _(Datalogisk Institut KÃ¸benhavns Universitet) at the department of computer science at the University of Copenhagen in Copenhagen, Denmark_. Commonly referred to as simply **"Diku"**, the game was greatly inspired by **AberMUD**, though **Diku** became one of the first multi-usergames to become popular as a freely-available program for its gameplay and similarity to **Dungeons & Dragons**.

**AberMUD** was the first popular open source MUD. It was named after the town _Aberystwyth_, in which it was written. The first version was written in **B** by **Alan Cox**, **Richard Acott**, **Jim Finnis**,and **Leon Thrane** based at **University of Wales**, _Aberystwyth_ for an old **Honeywell** mainframe and opened in 1987.


### --{SMAUG}-- Releases

Version **2.0** is HERE! With many bugfixes as well as important and useful new code features.

 * **[--{smaug}-- Release 2.0] (https://github.com/smaugmuds/_smaug_/releases/tag/2.0)** **Â¡Launch!**

#### Contents of the Release

	- src:              Source files.
	- doc:              Documentation.
	- bin:	            Init Scripts.
	- etc:              smaugd Script Config Files.
	- etc/init.d:       smaugd SysV Scripts.
	- man:              Man Pages.
	- data/area:        Realm Area files.
	- data/player:      Player files (initially empty).
	- data/backup:      Backup Player files (initially empty).
	- data/deleted:     Deleted Player files (initially empty).
	- data/vault:	    Guilds Vaults
	- data/races:	    Races files.
	- data/gods:        God players.
	- data/deity:	    Realm Deities.
	- data/councils:    Councils files.
	- data/classes:     Classes files.
	- data/clans:       Clans files.
	- data/boards:      Boards files.
	- data/corpses:     Death players corpses (initially empty).
	- data/houses:      Player house files.
	- data/mudprogs:    MUD Program files.
	- data/system:      MUD Game System files.
	- data/building:    Building System files.
	- data/classes:     Player Class files.
	- data/color:       MUD Color files.

#### Features

Here are just some of the extra features --{SMAUG}-- has:

	- Full featured, bug-free command-line online building
	- (including shops, resets, repairshops, etc)
	- Optional easy to use menu-based online building
	- Damageable and repairable equipment
	- MUDPrograms/scripts (mob, obj and room programs)
	- Monsters that will remember you, hunt you down, and summon you
	- Levers, switches, pullchains, buttons and traps
	- Searchable hidden objects and doors
	- More flexibility to command syntax
	- Delayed teleport rooms, tunnels
	- Clans, pkilling, guilds, orders and councils
	- Revamped experience and skill/spell learning system
	- Unlimited bulletin boards that can be added online
	- Private mail and comments on players
	- Automatic daily scheduled rebooting
	- Advanced immortal heirarchy
	- Complicated monsters with stats, attacks, defenses and bodyparts
	- Resistances, Immunities and Susceptibilities
	- More spells, more skills, more classes, more races
	- Weapon proficiencies
	- Pipes and herb smoking
	- Full featured container commands... fill, empty, etc
	- Extra exits - ne, nw, se, sw and more -
	- Special exit flags, floorless rooms (falling)
	- More room, mob and object flags, class restricted items
	- Languages, improved string hashing code
	- All kinds of 'spelled equipment' support
	- A new flexible spell system that allows online creation and editing
	- Object grouping, items hidden under other items, layered clothing
	- The ability to dig up and bury items
	- Player corpses are saved in case of crash or reboot
	- Stable code with hundreds of bug fixes
	- A fully configurable deity system
	- Many new admin commands such as an extensive ban, immhost ipcompare and more
	- Polymorph code that can be added, edited and deleted online
	- Different prompts when you are fighting and when you are not
	- Pets saving with their owners
	- Pet Finding.
	- New Healer: set the healer flag on a mob of your choice.
	- Configurable color codes online
	- Projectiles are fully supported
	- Extended bit vectors for when 32 are just not enough
	- SHA256 player passwords encryption
	- MUD Mapper and Planes
	- Dump Objects and Mobs to files
	- Mounts and much much more
	- Liquids & Mixtures System
	- MXP Support (MUD Extension Protocol)
	- Alias Support

Changes made to areas:

	- Major changes to the reset system to fix the problems with the nested put resets.
	- All of the stock areas in this package have been updated to support the new format.
	- Will not be backward compatible with any SMAUG mud.
	- Areas written for Smaug 1.02a, 1.4, and 1.4a should import without a problem and remain compatible.

Realm Areas:

	- astral.are chapel.are   dwarven.are  gallery.are  grave.are     haon.are
	- limbo.are  midennir.are newdark.are  pixie.are    redferne.are  srefuge.are
	- Build.are  daycare.are  export.are   gods.are     grove.are     help.are
	- manor.are  newacad.are  newgate.are  plains.are   sewer.are     unholy.are

#### --{SMAUG}-- ChangeLog

	* SMAUG 1.8b -> --{SMAUG}-- 2.0

		- Removed all InterMud code: IMC2 & ICE(D)
		- Conversion to GNU Autotools
		- Configure checks for cc, gcc, g++, (g)awk, -ldl, -lz, -lcrypt, basic #C libraries, headers and functions
		- Indented code with GNU style.
		- Server logs now goes to system directory "$(localstatedir)/log/smaug/"
		- "LOG_DIR" variable in "mud.h" is now obsolete and has been changed to "LOGDIR" in "config.h"
		- Init Script Daemons, Monitor and configuration files: smaugd.conf & smaugd & smaugd-monitor
		- Updated: Documentation
		- Fixed some bugs.


#### Supported Operating Systems

	- Linux
	- BSD
	- Solaris
	- Windows (CYGWIN)
	- MAC

 
### Authors and Contributors

**--{SMAUG}-- 2.0** by **Antonio Cao** ([@burzumishi] (https://github.com/burzumishi))
Wed Jun 11 14:49:27 CEST 2014

Feel free to contribute with **--{SMAUG}-- in GitHub**.


### Support or Contact

Having trouble with **--{SMAUG}--**?

First, check out the documentation at **[--{SMAUG}-- Wiki] (https://github.com/smaugmuds/xmerc/wiki)** or send an Issue and we'll help you sort it out.

We rewrote all of the documentation files for the **--{SMAUG}-- 2.0 Release**, and have been keeping them up-to-date since.

Also check the **'wizhelp'**, **'help'**, **'hlist'** commands and read the **'help'** descriptions for the individual immortal commands.

When you write us, we need to know what kind of machine you're running on.  If you can give us specific information about the problem, that helps too.

Specific information means: an adb or gdb stack trace (if you're reporting a crash), or a syslog with the relevant commands logged.  The player files are ascii files -- dump them into your mail message too.

You can also read the Documentation in the **'doc'** directory.



### Copyright and License

**Diku Mud** is copyright (C) 1990, 1991 by Sebastian Hammer, Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.  Their license agreement is in the file **'license.doc'**.

**Merc Diku Mud** is a derivative of the original **Diku Mud** and is subject to their copyright and license agreement.  **Merc Diku Mud** contains substantial enhancements to **Diku Mud**.  These enhancements are copyright 1992, 1993 by Michael Chastain, Michael Quan, and Mitchell Tse.  Our license agreement is in **'license.txt'**.

The license terms boil down to this: the people who wrote this mud want due credit for their work.

**MERC Industries** had officially disolved October 13, 1993.  **MERC 2.2** is a compilation of various bug and typo fixes along with a few new worlds.  **MERC 2.2** also has a variable line pager, MOBPrograms, and player settable prompt.

**SMAUG** is a derivative of the version **2.1** distribution of **Merc Diku Mud**. **SMAUG** contains substantial enhancements to **Merc Diku Mud**, including some features of the original **Diku Mud** that were removed by the Merc team.

**--{SMAUG}--** is a derivative of the original **SMAUG** and is subject to their copyright and license agreement. **--{SMAUG}--** contains substantial enhancements to **SMAUG**.  These enhancements are copyright 2014-2015 by **Antonio Cao** ([@burzumishi] (https://github.com/burzumishi)) under the **'GNU GPL v2 LICENSE'**.

**Thanks to the SMAUG Code Team!**

==============

_**--{SMAUG}--** (c) 2014-2015_
