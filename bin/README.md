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
    |                              README                                 |
    |_____________________________________________________________________|
    |                                                                     |
    |                            --(bin)--                                |
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

**--{SMAUG 2.0}--** is an enhancenment of **SMAUG 1.8b**, this project was started to have an stable source code structure, which will build, install and run in most modern distributed system environments. **--{SMAUG 2.0}--** is ready to add, enable and disable new code snippets using the new **GNU Autotools** build system. Many bug fixes and additions were made to the **SMAUG 1.8b** source code, always trying to keep the compatibility with **SMAUG** previous revisions. All path definitions for data files and directories were modified, **--{SMAUG}--** installation is now stored into a modern system directory distribution. Old "**startd**" script was replaced with "**smaugd**" and "**smaugd-monitor**", these scripts will start up and keep your MUD server running. If the **--{SMAUG}--** server process fails, "**smaugd-monitor**" will respawn it again. To know more about all these new changes, take a look at the section **--{SMAUG}-- ChangeLog**.

**SMAUG 1.8b**, is an fixed **SMAUG 1.4a** which is a heavily modified derivative of the **Merc 2.1** distribution. **SMAUG 1.4a** is close to six times the size of **Merc 2.1**, and has major changes to the inner workings of the code, hence making porting of code from other Diku-derivatives non-trivial to the non-coder. Due to the ability to edit most everything online, you will most likely find the online help pages more informative. **SMAUG** is the base code that runs the popular **Realms of Despair** MUD. Through the years it has been tweaked, modified, and enhanced to become one of the most feature packed branches of the **MERC** family of codebases. 

**SMAUG** is a derivative of **Merc Diku Mud** is a **Diku Mud** with many enhancements and contributions. It was written in 1990 and 1991 by **Sebastian Hammer**, **Tom Madsen**, **Katja Nyboe**, **Michael Seifert**, and **Hans Henrik StÃŠrfeldt** at **DIKU** _(Datalogisk Institut KÃžbenhavns Universitet) at the department of computer science at the University of Copenhagen in Copenhagen, Denmark_. Commonly referred to as simply **"Diku"**, the game was greatly inspired by **AberMUD**, though **Diku** became one of the first multi-usergames to become popular as a freely-available program for its gameplay and similarity to **Dungeons & Dragons**.

**AberMUD** was the first popular open source MUD. It was named after the town _Aberystwyth_, in which it was written. The first version was written in **B** by **Alan Cox**, **Richard Acott**, **Jim Finnis**,and **Leon Thrane** based at **University of Wales**, _Aberystwyth_ for an old **Honeywell** mainframe and opened in 1987.


### **--{SMAUG}--** Releases

**--{SMAUG}--** version **2.0** is HERE! With many bugfixes as well as important and useful new code and build features.

 * **[--{SMAUG 2.0}-- Release] (https://github.com/smaugmuds/_smaug_/releases/tag/2.0) ¡Launch!**

#### Contents of the Release

	- src:              Source files.
	- bin:	            Init Scripts.
	- etc:              smaugd Script Config Files.
	- etc/init.d:       smaugd Init Scripts.
	- doc:              Documentation.
	- man:              Man Pages.
	- data/area:        Realm Area files.
	- data/backup:      Backup Player files (initially empty).
	- data/boards:      Boards files.
	- data/building:    Building System files.
	- data/clans:       Clans files.
	- data/classes:     Classes files.
	- data/councils:    Councils files.
	- data/corpses:     Death corpses (initially empty).
	- data/deity:	    	Realm Deities.
	- data/deleted:     Deleted Player files (initially empty).
	- data/houses:      Houses files.
	- data/gods:        God players.
	- data/mudprogs:    MUD Program files.
	- data/player:      Player files (initially empty).
	- data/system:      MUD Game System files.
	- data/races:	    	Races files.
	- data/vault:	    	Guilds Vaults


#### Features

Here are just some of the features **--{SMAUG}--** has:

	- Full featured, bug-free command-line online building (including shops, resets, repairshops, etc)
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
	- Configurable color codes online
	- Projectiles are fully supported
	- Extended bit vectors for when 32 are just not enough
	- SHA256 player passwords encryption
	- MUD Mapper and Planes
	- Mounts and much much more
	- Liquids & Mixtures.
	- Banking System.
  - Player marriage system.

Realm Areas:
```
    +---------------------------+-------------+
    |            Area           |    Level    |
    +---------------------------+-------------+
    |   Spectral Gate           |    0 - 60   |
    |   New Darkhaven           |    0 - 60   |
    |   Darkhaven Academy       |    1 - 5    |
    |   Dwarven Daycare         |    1 - 5    |
    |   Pixie Forest            |    1 - 5    |
    |   Tullfuhrzky Manor       |    0 - 10   |
    |   Graveyard               |    5 - 10   |
    |   Haon Dor                |    5 - 10   |
    |   Miden'nir               |    5 - 15   |
    |   Unholy Grounds          |    5 - 15   |
    |   Holy Grove              |    5 - 20   |
    |   Shattered Refuge        |    5 - 20   |
    |   Dragon Exports          |    5 - 25   |
    |   Sewer                   |    5 - 30   |
    |   Dwarven Village         |   10 - 35   |
    |   Darkhaven Art Gallery   |   10 - 50   |
    |   Chapel Catacombs        |   15 - 25   |
    |   The Astral Plane        |   15 - 35   |
    |   Redferne's Residence    |   20 - 30   |
    |   Build Interface Stuff   |    0 - 60   |
    |   Gods                    |    0 - 60   |
    |   Limbo                   |    0 - 60   |
    +---------------------------+-------------+
```


#### **--{SMAUG}--** ChangeLog

	* Changes from **SMAUG 1.8b** to **--{SMAUG 2.0}--**:
```

		- Moved "KEY" definition to "mud.h".
		- Added GNU Gettext support.
		- Added Manteinance Message Aplication (announce.c).
		- Required runtime data directories moved to "data".
		- Renamed "services.c" to "win32srv.c".
		- Renamed "comm.c" to "smaug.c".
		- Removed all InterMud code: "IMC2" & "ICE(D)". If you want Intermud support maybe you can implement I3 or any other Intermud Chat source snippet.
		- Added "Banking" code snippet.
		- Added "Marriage" code snippet.
		- Fixed "Liquids & Mixtures" system, replaces old "do_drink", "do_empty" & "do_fill" from "misc.c" routines.
		- Fixed many bugs and redundant definitions.
		- Indented code with GNU style.
		- Init Script Daemon, Monitor and Configuration files: "smaugd.conf" & "smaugd" & "smaugd-monitor".
		- Updated documentation files.
		- Added "Man Page" files for "smaug" and "smaugd".
		- Conversion to "GNU Autotools" build system.
		- Added "PREFIX", "DESTDIR" & "RUNDIR" to "configure.ac", "smaug.h" and "mud.h".
		- "LOG_DIR" variable in "mud.h" is now obsolete and has been changed to "LOGDIR" in "smaug.h", all server log files will be stored to "$(localstatedir)/log/smaug/" system directory.
		- Installed files are now splitted in several libraries and binaries using "Ranlib & Libtool": "libsmaug-act.so", "libsmaug-player.so", "libsmaug-mudprog.so", "libsmaug-info.so", "libsmaug-realm.so", "smaug", "smaugd" & "smaugd-monitor".
		- Configure checks for "cc", "gcc", "g++", "(g)awk", "-ldl", "-lz", "-lcrypt", "-lssl", "basic C# libraries", "C# headers & functions", "bash", "env", "perl".
		- Configure options to include required definitions for enabling or disabling code snippets and flags:

				Configured --{SMAUG}-- build flags:

				 * Time Format fix:     <no>
				 * RE_EXEC fix:         <yes>
				 * Profiling flags:     <no>
				 * Developer flags:     <no>
				 * Performance flags:   <yes>

				Configured --{SMAUG}-- MUD Server basic features:

				 * Password Encryption: <yes>
				 * Request Support:     <no>
				 * Planes:              <yes>
				 * Who arguments:       <no>
				 * Honour Code:         <yes>
				 * Housing:             <yes>
				 * House Mobs:          <yes>
				 * Extended Rooms:      <yes>
				 * Act Comm Scramble:   <no>
				 * Liquids System:      <yes>

				Configured --{SMAUG}-- MUD Server extra features:

				 * SMAUG 2.0:           <yes>
				 * New Score Tables:    <yes>
				 * Show Stance Info:    <yes>
				 * Banking:             <yes>
				 * Marry:               <yes>
				 * Player Bleeding:     <yes>
				 * DNS Resolver:        <yes>
				 * Web Server:          <yes>
				 * HotBoot:             <no>
```

#### TODO

This is a list of TODO stuff:

	- New weather system.
	- Implement MXP, MSSP & MCCP protocols.
	- More useful code snippets.

#### Supported Operating Systems

	- Linux
	- BSD
	- Solaris
	- Windows (CYGWIN)
	- MAC

 
### Authors and Contributors

**--{SMAUG 2.0}--** by **Antonio Cao** ([@burzumishi] (https://github.com/burzumishi))
Wed Jun 11 14:49:27 CEST 2014

Feel free to contribute with **--{SMAUG}-- in GitHub**.


### Support or Contact

Having trouble with **--{SMAUG}--**?

First, check out the documentation at **[--{SMAUG}-- Wiki] (https://github.com/smaugmuds/xmerc/wiki)** or send an Issue and we'll help you sort it out.

We rewrote all of the documentation files for the **--{SMAUG}-- 2.0 Release**, and have been keeping them up-to-date since.

Also check the **'wizhelp'**, **'help'**, **'hlist'** commands and read the **'help'** descriptions for the individual immortal commands.

When you write us, we need to know what kind of machine you're running on.  If you can give us specific information about the problem, that helps too.

Specific information means: an **adb** or **gdb** stack trace (if you're reporting a crash), or a **syslog** with the relevant commands logged. The player files are ascii files dump them into your mail message too.

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

_**--{SMAUG}--** (c) 2014-2015 Antonio Cao (@burzumishi)_
