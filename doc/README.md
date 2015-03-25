##[XSMAUG] (http://smaugmuds.github.io/xsmaug)

###Introduction

```
 ############################################################################
 #                                                                          #
 #   X      X  ******* **    **  ******  **    **  ******                   #
 #    X    X  ******** ***  *** ******** **    ** ********       \\._.//    #
 #     X  X   **       ******** **    ** **    ** **             (0...0)    #
 #      XX    *******  ******** ******** **    ** **  ****        ).:.(     #
 #      XX     ******* ** ** ** ******** **    ** **  ****        {o o}     #
 #     X  X         ** **    ** **    ** **    ** **    **       / ' ' \    #
 #    X    X  ******** **    ** **    ** ******** ********    -^^.VxvxV.^^- #
 #   X      X *******  **    ** **    **  ******   ******                   #
 #                                                                          #
 # ------------------------------------------------------------------------ #
 # Ne[X]t Generation [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame  #
 # ------------------------------------------------------------------------ #
 # XSMAUG 2.5 (C) 2014-2015  by Antonio Cao @burzumishi                     #
 # ------------------------------------------------------------------------ #
 #                                  XSMAUG                                  #
 ############################################################################
```

**Ne[X]t Generation of [S]imulated [M]edieval [A]dventure Multi-[U]ser [G]ame**

**XSMAUG** is a multiplayer text-based role-playing game, which is a type of MUD. It can be used and modified to create your own game server.

**XSMAUG** is an enhancenment of **AFKMud 2.1.5**, the goal is to develop a new structure to build and run in modern distributed system environments. **AFKMud**, is an fixed **SMAUG 1.4a** which is a heavily modified derivative of the **Merc 2.1** distribution. Stock area zones from **SMAUG 1.4a** have been restored into **XSMAUG**.

**SMAUG 1.4a** is close to six times the size of **Merc 2.1**, and has major changes to the inner workings of the code, hence making porting of code from other Diku-derivatives non-trivial to the non-coder. Due to the ability to edit most everything online, you will most likely find the online help pages more informative. **SMAUG** is the base code that runs the popular **Realms of Despair** MUD. Through the years it has been tweaked, modified, and enhanced to become one of the most feature packed branches of the **MERC** family of codebases. 

**SMAUG** is a derivative of **Merc Diku Mud** is a **Diku Mud** with many enhancements and contributions. It was written in 1990 and 1991 by **Sebastian Hammer**, **Tom Madsen**, **Katja Nyboe**, **Michael Seifert**, and **Hans Henrik Stærfeldt** at **DIKU** _(Datalogisk Institut Københavns Universitet)—the department of computer science at the University of Copenhagen in Copenhagen, Denmark_. Commonly referred to as simply **"Diku"**, the game was greatly inspired by **AberMUD**, though **Diku** became one of the first multi-usergames to become popular as a freely-available program for its gameplay and similarity to **Dungeons & Dragons**.

**AberMUD** was the first popular open source MUD. It was named after the town _Aberystwyth_, in which it was written. The first version was written in **B** by **Alan Cox**, **Richard Acott**, **Jim Finnis**,and **Leon Thrane** based at **University of Wales**, _Aberystwyth_ for an old **Honeywell** mainframe and opened in 1987.


### XSMAUG Releases

The release of version **2.4** is HERE! With many bugfixes as well as important and useful new code features.

 * **[XSMAUG Phoenix Release 2.0] (https://github.com/smaugmuds/xsmaug/releases/tag/2.0)** **¡Launch!**
 * **[XSMAUG Phoenix Release 2.1] (https://github.com/smaugmuds/xsmaug/releases/tag/2.1)** **¡Update!**
 * **[XSMAUG Phoenix Release 2.2] (https://github.com/smaugmuds/xsmaug/releases/tag/2.2)** **¡Update!**
 * **[XSMAUG Phoenix Release 2.4] (https://github.com/smaugmuds/xsmaug/releases/tag/2.4)**    **¡NEW!**
 * **[XSMAUG Phoenix Release 2.5] (CURRENT GIT)

#### Contents of the Release

	- src:              Source files.
	- doc:              Documentation.
	- bin:							Init Scripts.
	- etc:  						XSMAUG Script Config Files.
	- etc/init.d: 			XSMAUG & HTTPD Server SysV Scripts.
	- man:							Man Pages.
	- tools:            Developer tools.
	- data/realm:       Realm Area files.
	- data/areaconvert:	Convert New Realm Area files.
	- data/aucvaults:   Auction Vault files.
	- data/player:      Player files (initially empty).
	- data/vault:	   		Guilds Vaults
	- data/races:	   		Races files.
	- data/gods:      	God players.
	- data/deity:	    	Realm Deities.
	- data/councils:    Councils files.
	- data/classes:     Classes files.
	- data/clans:       Clans files.
	- data/shops:       Clans Shops files.
	- data/boards:      Boards files.
	- data/corpses:     Death players corpses (initially empty).
	- data/houses:      Player house files.
	- data/mudprogs:    MUD Program files.
	- data/system:      MUD Game System files.
	- data/building:    Building System files.
	- data/classes:     Player Class files.
	- data/color:	    	MUD Color files.
	- data/hotboot:     Fast boot config files.
	- data/crash:       Crash files.
	- data/motd:        Message files.
	- data/web:         Web Site & MUD dumped HTML Files.
	- db/darkhaven.db:  Database Test File

#### Features

Here are just some of the extra features XSMAUG has:

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
	- MCCP and MSSP
	- New Weather System and Calendar
	- HotBoot!
	- Dump Objects and Mobs to files
	- Quest bits added.
	- Quest Master.
	- Banker: Set the banker flag on a mob of your choice.
	- Clans Shopkeepers
	- Mounts and much much more

Changes made to areas:

	- Major changes to the reset system to fix the problems with the nested put resets.
	- Major changes to the shops system.
	- All of the stock areas in this package have been updated to support the new format.
	- Will not be backward compatible with any SMAUG mud.
	- Areas written for Smaug 1.02a, 1.4, and 1.4a should import without a problem and remain compatible.

Realm Areas:

	- air.are         daycare.are     gate.are       hollywd.are      mirkwood.are  plains.are    sewer.are
	- anthill.are     donjon.are      gauntlet.are   immtrain.are     mobfact.are   prehist.are   shaolin.are
	- anvil.are       draconia.are    gilligan.are   jher.are         moria.are     pyramid.are   shire.are
	- astral.are      dwarfmines.are  goblins.are    juargan.are      mtdoom.are    rainfrst.are  srefuge.are
	- bazaar.are      dwarven.are     gods.are       keepvalor.are    newacad.are   rats.are      thalos.are
	- Build.are       eastern.are     grave.are      land.are         newdark.are   redferne.are  tree.are
	- canyon.are      elderwood.are   grove.are      lilith.are       olympus.are   rift.are      trollden.are
	- catacombs.are   entry.are       gtown.are      limbo.are        one.are       rshouse.are   troy.are
	- chapel.are      export.are      gwyn.are       magicschool.are  orchold.are   rtower.are    under.are
	- chapelgods.are  fantasia.are    gyard.are      mahntor.are      pblood.are    school2.are   unholy.are
	- cloudymt.are    forestpath.are  haon.are       manor.are        pground.are   seatears.are  valley.are
	- crystalmir.are  galaxy.are      haven.are      marsh.are        pit.are       sesame.are    void.are
	- damara.are      gallery.are     highcliff.are  midennir.are     pixie.are     seven.are     warkeep.are
	- warner.are      waymoot.are     weater.are     weed.are         weirdan.are   wyvern.are


#### ChangeLog

	* XSMAUG 2.4 -> XSMAUG 2.5

		- Init Script Daemons, Monitor and configuration files.
		- Some build fixes.

	* XSMAUG 2.2 -> XSMAUG 2.4

		- Conversion to GNU Package: configure, make, make install
		- Configure checks for cc, gcc, g++, (g)awk, -ldl, -lz, -lcrypt, -lgd, -lmysqlclient, basic c libraries, headers and functions
		- Indented code with GNU style.
		- "LOG_DIR" variable in "mud.h" is now obsolete and has been changed to "LOGDIR" in "config.h"
		- Server logs now goes to system directory "$(localstatedir)/log/xsmaug/"
		- Port XSMAUG from AFKMud 2.1.5.
		- Import Stock SMAUG 1.4a Areas
		- Changes in structure files and directories.
		- Fixed some bugs.
		- Added: Optional HTTP Server to test & publish MUD generated HTML pages with static content.
		- Added: "CMDF( do_version )" based in "config.h" defines.
		- Added: "CMDF( do_petfind )"
		- Added: xsmaug.conf & xsmaugd
		- Updated: "CMDF( do_webroom )" now it accepts "area.are" file as argument.
		- Updated: Tools
		- Updated: Documentation

	* XSMAUG 2.1 -> XSMAUG 2.2

		- Conversion to GNU Package: configure, make, make install
		- Configure checks for cc, gcc, g++, (g)awk, -ldl, -lz, -lcrypt, -lgd, -lmysqlclient, basic c libraries, headers and functions
		- Indented code with GNU style.
		- New structure.
		- Server logs now goes to system directory "$(localstatedir)/log/xsmaug/"
		- "LOG_DIR" variable in "mud.h" is now obsolete and has been changed to "LOGDIR" in "config.h"
		- Added: Command "dump" (do_dump)
		- Added: Banker (ACT_BANKER): (do_balance, do_deposit, do_withdraw)
		- Added: Quest Bits (do_abit, do_qbit) & New Quest Master
		- Added: Capture SIGTERM & More signals.
		- Added: Admin list (admin.lst)

	* XSMAUG 2.0 -> XSMAUG 2.1

		- Added: Command 'petfind' (do_petfind)
		- Added: New Healer: 'heal' (do_heal)
		- Updated: Makefile
		- Updated: Areas
		- Updated: Documentation
		- Updated: Tools

	* SMAUGFuss 1.9  -> XSMAUG 2.0

		- Added: xsmaug.conf & xsmaug-server
		- Added: New Makefile
		- Added: Name Change
		- Added: New Clases & Skills
		- Added: New Imported Areas
	

#### Supported Operating Systems

	- Linux
	- BSD
	- Solaris
	- Windows
	- MAC

 
### Authors and Contributors

**XSMAUG Phoenix Release** by **Antonio Cao** ([@burzumishi] (https://github.com/burzumishi))
Wed Jun 11 14:49:27 CEST 2014

Feel free to contribute with **XSMAUG in GitHub**.


### Support or Contact

Having trouble with **XSMAUG**?

First, check out the documentation at **[XSMAUG Wiki] (https://github.com/smaugmuds/xmerc/wiki)** or send an Issue and we’ll help you sort it out.

We rewrote all of the documentation files for the **XSMAUG 2.0 Phoenix Release**, and have been keeping them up-to-date since.

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

**XSMAUG** is a derivative of the original **SMAUG** and is subject to their copyright and license agreement.  **XSMAUG** contains substantial enhancements to **SMAUG**.  These enhancements are copyright 2014-2015 by **Antonio Cao** ([@burzumishi] (https://github.com/burzumishi)) under the **'GNU GPL v2 LICENSE'**.

**Thanks to the SMAUG Code Team!**

==============

_**XSMAUG** (c) 2014-2015_
