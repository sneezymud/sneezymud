##########################################################################
##
## SneezyMUD - All rights reserved, SneezyMUD Coding Team
##
##########################################################################

###############################################################################
#                                 Pre-Defines                                 #
###############################################################################
NAME = sneezy.2
VERSION_FILE = lib/txt/version.new
NAME2 = sneezy.insure

###############################################################################
#                                Type-Defines                                 #
###############################################################################
# Compiler Type
# CC = checkerg++
# CC = /usr/pubsw/bin/g++
# GPROFF=1    # Use with care, mud stays up for about 45 minutes with this on.
# USEINSURE=1 # Uncomment this if you uncomment the CC = insure above
# USECRYPT=1  # Uncomment to have all the crypt stuff
# BATOPR = 1    # Uncomment to test something Batopr is testing

ifdef USEINSURE
  CC = insure
else
  CC = /usr/bin/g++
endif

###############################################################################
#                    Base Values (Do Not Modify/Add Here)                     #
###############################################################################
CFLAGS = # empty


###############################################################################
#                                   CFLAGS                                    #
###############################################################################
# CFLAGS +=-static      # link statically
# CFLAGS +=-Zvm         # insure verbose mode (spamy)
ifdef GPROFF
CFLAGS +=-pg            # add gprof mapping
endif
CFLAGS +=-g             # debugger
# CFLAGS +=-ggdb        # force debugger into gdb format (if not default)
# CFLAGS +=-gstabs+     # add stabs extensions
# CFLAGS +=-DOSF        # compiling on an OSF/1 system
# CFLAGS +=-DSUN        # compiling on SunOs UNIX
# CFLAGS +=-DSOLARIS    # compiling on Solaris UNIX
CFLAGS +=-DLINUX        # compiling on Linux
# CFLAGS +=-DSLOW       # compiling on a slow processor machine
CFLAGS +=-DFAST         # compiling on a fast processor machine
CFLAGS +=-DSNEEZY2000   # activation of sneezy version 5.2

ifdef BATOPR
#  CFLAGS +=-O4            # optimize code
  OBJDIR = bat_objs/
else
  ifdef USEINSURE
#    CFLAGS +=-O4            # optimize code
#  with the current .psrc file the O4 flag is ignored
#  see the Parasoft Manual for details

    OBJDIR = insure_objs/
  else
    CFLAGS +=-O4            # optimize code
    OBJDIR = objs/
  endif
endif

#CFLAGS +=-O3           # optimize code
#CFLAGS +=-fhandle-exceptions
#CFLAGS +=-frtti
# These are valid, but removed for insure
ifndef USEINSURE
CFLAGS +=-Wall                  # all rec. warnings shown (includes all above)
CFLAGS +=-Wpointer-arith        # looks for things that depend on sizeof()
CFLAGS +=-Wcast-qual            # Warns about using casting to remove const
CFLAGS +=-Wcast-align           # a pointer cast changes alignment
CFLAGS +=-Wwrite-strings        # looks for formal const char* sent to char*
CFLAGS +=-Wconversion           # bad conversions
CFLAGS +=-Wstrict-prototypes    # functions declared sans argument types
CFLAGS +=-Wmissing-prototypes   # global functions without a prototype
CFLAGS +=-Wmissing-declarations # global function without a declaration
CFLAGS +=-Wnested-externs       # extern found within a func
CFLAGS +=-Woverloaded-virtual   # virtual without a declaration in base class
CFLAGS +=-Werror                # turns a warning into an error
endif
# **** Aggregates are bad, but are sometimes necessary.  We use them though.
# CFLAGS +=-Waggregate-return # checks for structures passed on the stack
# **** disabled as is invalid warning on LINUX (?)
# CFLAGS +=-Wenum-clash       # conversion between different enum types
# **** disabled due to shadows in header file functions
# CFLAGS +=-Wshadow           # check if local variable shadows other variable
# **** disabled due to redundancy in header file functions
# CFLAGS +=-Wredundant-decls  # declaration done more than once
# **** the STL map header file has a non-inlinable function or two
# CFLAGS +=-Winline           # looks for non-inlineable inlines
# postgresql header files
#CFLAGS +=-I/usr/include/postgresql
CFLAGS +=-DUSE_SQL=1

###############################################################################
#                                   LDFLAGS                                   #
###############################################################################
LDFLAGS=-g # enable core debugging
LDFLAGS +=-L/usr/lib # specify this directory to look in
# LDFLAGS +=-L/usr/pubsw/lib # specify this directory to look in
# **** libg++ and libm seem to be equivalent for Gnu 2.7.2 and before
# **** The standard C++ libraries
# LDFLAGS +=-Bdynamic -lg++
# LDFLAGS +=-bstatic -lg++
# LDFLAGS +=-lg++
# LDFLAGS +=/usr/pubsw/lib/libg++.a # enable for insight -j
# LDFLAGS +=/usr/lib/libg++.a # enable for insight -j
LDFLAGS +=-lstdc++
# LDFLAGS +=/usr/lib/libstdc++.so.2.8
# LDFLAGS +=-bstatic -lstdc++
# LDFLAGS +=/usr/lib/libstdc++.a
# LDFLAGS +=/usr/lib/libstdc++-libc6.1-1.a.2
# **** not sure what this was for, but used to use it
# LDFLAGS +=-d
# **** sometimes necessary for sockets code
# LDFLAGS += -lsocket -lnsl
# **** sometimes necessary for crypt code
ifdef USECRYPT
LDFLAGS +=/usr/lib/libcrypt.a
# LDFLAGS +=-Bdynamic -lcrypt
# LDFLAGS +=-Bstatic -lcrypt
LDFLAGS +=-lcrypt
LDFLAGS +=/usr/pubsw/lib/libcrypt.a # enable for insight
else
LDFLAGS +=/usr/lib/libcrypt.a # enable for insight
endif
ifdef GPROFF
LDFLAGS +=-pg #              gprof mapping
endif
#LDFLAGS +=-fhandle-exceptions
# postgresql database lib
#LDFLAGS +=-lpq++
# mysql database lib
LDFLAGS +=-lmysqlclient

###############################################################################
##                        Actual Compiling Starts Here                       ##
###############################################################################

OBJ = actions.o ai_commands.o ai_reactions.o ai_responses.o ai_utility.o \
	alignment_deity.o ansi.o \
	being.o blackjack.o body.o \
	bounty_hunter.o breath.o \
	cards.o client.o \
	cmd_attribute.o cmd_show.o \
	cmd_bash.o cmd_bodyslam.o cmd_charge.o cmd_chop.o \
	cmd_compare.o cmd_consider.o \
	cmd_deathstroke.o cmd_disarm.o cmd_dissect.o \
	cmd_doorbash.o cmd_egotrip.o \
	cmd_feigndeath.o cmd_get.o cmd_grapple.o cmd_headbutt.o \
	cmd_help.o cmd_jump.o \
	cmd_kick.o cmd_kneestrike.o cmd_look.o cmd_low.o \
	cmd_mend_limb.o cmd_message.o \
	cmd_news.o \
	cmd_quest.o cmd_quivpalm.o cmd_rename.o cmd_rescue.o cmd_save.o \
	cmd_score.o cmd_sedit.o cmd_set.o cmd_spin.o cmd_stab.o cmd_stat.o cmd_steal.o \
	cmd_stomp.o cmd_testcode.o cmd_testfight.o cmd_trophy.o\
	cmd_visible.o cmd_who.o \
	cmd_zonefile.o \
	cmd_zones.o color.o colorstring.o \
	combat.o comm.o commodity.o components.o connect.o constants.o \
	craps.o crazyeights.o create_engine.o \
	create_mobs.o create_objs.o create_rooms.o crit_combat.o damage.o \
	db.o disc_adventuring.o disc_aegis.o disc_afflictions.o disc_air.o \
	disc_alchemy.o disc_animal.o disc_blunt.o disc_brawling.o \
	disc_cleric.o disc_combat.o disc_cure.o disc_deikhan.o \
	disc_earth.o disc_fattacks.o disc_fire.o \
	disc_hand_of_god.o disc_hth.o disc_leverage.o disc_looting.o \
	disc_mage.o disc_meditation.o disc_mindbody.o disc_monk.o \
	disc_mounted.o disc_murder.o disc_physical.o disc_nature.o \
	disc_pierce.o disc_ranged.o disc_ranger.o disc_shaman.o disc_shaman_armadillo.o \
	disc_shaman_frog.o disc_shaman_spider.o disc_shaman_control.o \
	disc_slash.o disc_smythe.o disc_sorcery.o disc_spirit.o \
	disc_survival.o disc_theology.o disc_thief.o disc_warrior.o \
	disc_water.o disc_wrath.o discipline.o disease.o doors.o drawpoker.o \
	drinkcon.o drug_container.o drug.o edit.o enum.o equip.o \
	expandable_container.o faction.o flame.o food.o fuel.o gaining.o \
	gemstone.o general_weapon.o 

OBJ2 = gin.o handler.o hearts.o hospital.o immortal.o immunity.o \
	indexdata.o info.o inventory.o jewelry.o key.o keyring.o light.o \
	limbs.o limits.o loadset.o luck.o magic_item.o magic_off.o \
	magic_skills.o magicutils.o mail.o makecorpse.o materials.o \
	meeting.o mob_loader.o mobact.o money.o monster.o movement.o \
	multiclass.o newbie.o note.o obj.o \
	obj_armor.o obj_armor_wand.o obj_arrow.o obj_appliedsub.o \
	obj_audio.o \
	obj_bag.o obj_bandaid.o obj_base_clothing.o \
	obj_base_container.o obj_base_corpse.o \
	obj_base_cup.o obj_base_light.o obj_base_weapon.o obj_bed.o \
	obj_board.o obj_boat.o obj_book.o obj_bow.o \
	obj_chest.o obj_corpse.o \
	obj_open_container.o \
	obj_scroll.o obj_seethru.o obj_smoke.o obj_spellbag.o \
	obj_staff.o obj_statue.o obj_symbol.o \
	obj_table.o obj_tool.o obj_trap.o \
	obj_trash.o obj_treasure.o obj_tree.o \
	obj_vial.o \
	obj_wand.o obj_window.o obj_worn.o \
	offense.o oldlimbs.o oldrace.o \
	opal.o opinion.o organic.o other.o other_obj.o parse.o peelpk.o \
	pen.o periodic.o pets.o physics.o player_corpse.o player_data.o \
	poker.o pool.o portal.o posse.o potion.o quiver.o race.o range.o \
	rent.o repair.o response.o riding.o room.o \
	shop.o show.o signals.o skill_dam.o skills.o \
	slots.o socket.o sound.o spec_mobs.o \
	spec_mobs_doppleganger.o spec_mobs_earthquake_digger.o \
        spec_mobs_goring.o \
	spec_objs.o spec_objs_jambiya.o \
	spec_objs_lightning_rod.o \
	spec_objs_blind.o spec_objs_manadrain.o \
	spec_rooms.o \
        spec_rooms_sleeptag.o \
	spell_info.o \
	spell_num.o spell_parser.o \
	spellstore.o \
	spelltask.o \
	statistics.o stats.o structs.o systemtask.o \
	talk.o task.o task_attune.o task_lightfire.o task_fishing.o task_sacrifice.o task_scribe.o \
	task_extinguish_my_ass.o \
	task_skin.o task_trap.o task_brew.o task_meditate.o task_search.o \
	task_sleep.o task_charge.o task_penance.o task_seekwater.o \
	task_smythe.o task_dull.o task_picklock.o task_sharpen.o \
	task_spell_friends.o task_get.o task_rest.o task_sit.o task_track.o \
	task_whittle.o task_stavecharge.o thing.o toggle.o trap.o \
	task_trance_of_blades.o \
	utility.o weather.o sys_loot.o \
	wiz_data.o wiz_powers.o

#	rewrite_obj_file.o \

OBJ := $(sort $(addprefix $(OBJDIR), $(OBJ)))
OBJ2 := $(sort $(addprefix $(OBJDIR), $(OBJ2)))

ifdef USEINSURE
all: $(NAME2)
#insure.o
#(CC) -o jjb insure.o $(LDFLAGS)

else
all: $(NAME)
endif

reverse : $(OBJ2) $(OBJ)

$(NAME) : $(OBJ) $(OBJ2)
	@echo \*\*\* Making Final Link: $(NAME)
	@$(CC) -o $(NAME) $(OBJ) $(OBJ2) $(LDFLAGS)
	@rm -f $(VERSION_FILE)
	@echo "`date`" > $(VERSION_FILE)
	@echo \*\*\* Make Process Complete
	@echo "`date`"

$(NAME2) : $(OBJ) $(OBJ2)
	@echo \*\*\* Making Final Link: $(NAME2)
	@$(CC) -o $(NAME2) $(OBJ) $(OBJ2) $(LDFLAGS)
	@rm -f $(VERSION_FILE)
	@echo "`date`" > $(VERSION_FILE)
	@echo \*\*\* Insure Compile Process Complete
	@echo "`date`"

clean:
	/bin/rm -f $(OBJDIR)*.o

GEN_HEADS    = structs.h obj.h thing.h being.h body.h room.h person.h \
               monster.h client.h low.h obj2.h trap.h mobAI.h weather.h \
               extern.h parse.h faction.h handler.h spells.h disease.h \
               spell2.h response.h db.h connect.h comm.h rent.h spec_objs.h \
               stdsneezy.h account.h spec_mobs.h spec_rooms.h discipline.h \
               race.h ansi.h materials.h task.h charfile.h \
               skills.h toggle.h limbs.h immunity.h stats.h \
               sound.h enum.h wiz_powers.h cmd_message.h create.h cmd_dissect.h

CASINO_HEADS = games.h crazyeights.h drawpoker.h

DISC_HEADS   = disc_adventuring.h disc_aegis.h disc_afflictions.h \
               disc_air.h disc_animal.h disc_blunt.h \
               disc_brawling.h disc_meditation.h disc_combat.h disc_cleric.h \
               disc_cures.h disc_deikhan.h disc_deikhan_aegis.h \
               disc_deikhan_cures.h disc_deikhan_fight.h disc_mounted.h \
               disc_deikhan_wrath.h disc_earth.h  \
               disc_fire.h disc_hand_of_god.h disc_hth.h  \
               disc_leverage.h disc_fattacks.h disc_looting.h disc_lore.h  \
               disc_mage.h disc_monk.h disc_murder.h  \
               disc_pierce.h disc_plants.h disc_physical.h \
               disc_poisons.h disc_ranged.h disc_ranger.h disc_nature.h \
               disc_ranger_fight.h disc_shaman.h disc_shaman_armadillo.h \
	       disc_shaman_frog.h disc_shaman_skunk.h disc_shaman_spider.h \
               disc_shaman_alchemy.h disc_shaman_control.h \
               disc_smythe.h disc_sorcery.h \
               disc_spirit.h disc_stealth.h disc_survival.h   \
               disc_mindbody.h disc_theology.h disc_thief.h  \
               disc_thief_fight.h disc_totem.h disc_traps.h  \
               disc_warrior.h disc_water.h disc_wrath.h

$(OBJDIR)%.o : %.cc $(GEN_HEADS)
	@echo Making \"$<\"
	@$(CC) $(CFLAGS) -c $< -o $@

%.o : %.cc $(GEN_HEADS)
	@echo Making \"$<\"
	@$(CC) $(CFLAGS) -c $< -o $(OBJDIR)$@

$(OBJDIR)ai_commands.o: combat.h
$(OBJDIR)ai_reactions.o: combat.h
$(OBJDIR)ai_responses.o: combat.h parse.h
$(OBJDIR)ai_utility.o: combat.h
$(OBJDIR)blackjack.o: $(CASINO_HEADS)
$(OBJDIR)bounty_hunter.o: shop.h
$(OBJDIR)cards.o: $(CASINO_HEADS)
$(OBJDIR)client.o: statistics.h
$(OBJDIR)cmd_bash.o: combat.h
$(OBJDIR)cmd_bodyslam.o: combat.h
$(OBJDIR)cmd_charge.o: combat.h
$(OBJDIR)cmd_chop.o: disc_monk.h combat.h
$(OBJDIR)cmd_compare.o: shop.h
$(OBJDIR)cmd_deathstroke.o: combat.h
$(OBJDIR)cmd_disarm.o: combat.h
$(OBJDIR)cmd_dissect.o: components.h disc_adventuring.h
$(OBJDIR)cmd_doorbash.o: disc_warrior.h combat.h
$(OBJDIR)cmd_egotrip.o: disc_fire.h disc_aegis.h
$(OBJDIR)cmd_get.o: games.h
$(OBJDIR)cmd_kick.o: combat.h
$(OBJDIR)cmd_kneestrike.o: combat.h
$(OBJDIR)cmd_feigndeath.o: combat.h
$(OBJDIR)cmd_grapple.o: combat.h
$(OBJDIR)cmd_headbutt.o: combat.h
$(OBJDIR)cmd_help.o: components.h help.h statistics.h
$(OBJDIR)cmd_look.o: games.h
$(OBJDIR)cmd_low.o: statistics.h combat.h
$(OBJDIR)cmd_news.o: statistics.h
$(OBJDIR)cmd_quivpalm.o: combat.h
$(OBJDIR)cmd_sedit.o: combat.h dirsort.h
$(OBJDIR)cmd_set.o: statistics.h
$(OBJDIR)cmd_spin.o: combat.h
$(OBJDIR)cmd_stab.o: combat.h
$(OBJDIR)cmd_steal.o: disc_looting.h
$(OBJDIR)cmd_stomp.o: combat.h
$(OBJDIR)cmd_testfight.o: spelltask.h
$(OBJDIR)cmd_who.o: statistics.h games.h
$(OBJDIR)combat.o: combat.h statistics.h range.h mail.h spelltask.h shop.h
$(OBJDIR)comm.o: task.h socket.h sneezy_signals.h systemtask.h
$(OBJDIR)insure.o: task.h socket.h sneezy_signals.h systemtask.h
$(OBJDIR)commodity.o: shop.h
$(OBJDIR)components.o: components.h shop.h
$(OBJDIR)connect.o: statistics.h mail.h $(CASINO_HEADS) socket.h
$(OBJDIR)craps.o: $(CASINO_HEADS)
$(OBJDIR)crazyeights.o: $(CASINO_HEADS)
$(OBJDIR)create_mobs.o: mail.h statistics.h combat.h
$(OBJDIR)create_engine.o: create_engine.h
$(OBJDIR)create_objs.o: dirsort.h components.h statistics.h
$(OBJDIR)create_rooms.o: room.h
$(OBJDIR)crit_combat.o: combat.h statistics.h
$(OBJDIR)damage.o: combat.h statistics.h
$(OBJDIR)db.o: components.h help.h board.h statistics.h mail.h loadset.h
$(OBJDIR)db.o: sys_loot.h
$(OBJDIR)disc_adventuring.o: disc_adventuring.h combat.h
$(OBJDIR)disc_aegis.o: disc_aegis.h combat.h
$(OBJDIR)disc_afflictions.o: disc_afflictions.h combat.h
$(OBJDIR)disc_air.o: disc_air.h spelltask.h combat.h
$(OBJDIR)disc_alchemy.o: disc_alchemy.h components.h combat.h spelltask.h
$(OBJDIR)disc_animal.o: disc_animal.h combat.h
$(OBJDIR)disc_azruzi.o: disc_azruzi.h combat.h
$(OBJDIR)disc_blunt.o: disc_blunt.h
$(OBJDIR)disc_brawling.o: disc_brawling.h
$(OBJDIR)disc_meditation.o: disc_meditation.h disc_combat.h
$(OBJDIR)disc_cleric.o: disc_cleric.h
$(OBJDIR)disc_combat.o: disc_combat.h components.h combat.h
$(OBJDIR)disc_cure.o: disc_cures.h combat.h spelltask.h
$(OBJDIR)disc_deikhan.o: disc_deikhan.h combat.h
$(OBJDIR)disc_earth.o: disc_earth.h combat.h spelltask.h
$(OBJDIR)disc_fire.o: disc_fire.h spelltask.h combat.h
$(OBJDIR)disc_hand_of_god.o: disc_hand_of_god.h combat.h
$(OBJDIR)disc_hth.o: disc_hth.h range.h combat.h
$(OBJDIR)disc_leverage.o: disc_leverage.h combat.h range.h
$(OBJDIR)disc_fattacks.o: disc_fattacks.h combat.h
$(OBJDIR)disc_looting.o: disc_looting.h combat.h
$(OBJDIR)disc_mage.o: disc_mage.h
$(OBJDIR)disc_monk.o: disc_monk.h combat.h
$(OBJDIR)disc_mounted.o: disc_mounted.h
$(OBJDIR)disc_murder.o: disc_murder.h combat.h spelltask.h
$(OBJDIR)disc_nature.o: disc_nature.h combat.h spelltask.h
$(OBJDIR)disc_shaman_armadillo.o: disc_shaman_armadillo.h combat.h spelltask.h
$(OBJDIR)disc_shaman_frog.o: disc_shaman_frog.h combat.h spelltask.h
$(OBJDIR)disc_shaman_spider.o: disc_shaman_spider.h combat.h spelltask.h
$(OBJDIR)disc_shaman_control.o: disc_shaman_control.h combat.h spelltask.h
$(OBJDIR)disc_physical.o: disc_pierce.h
$(OBJDIR)disc_pierce.o: disc_pierce.h
$(OBJDIR)disc_ranged.o: disc_ranged.h
$(OBJDIR)disc_ranger.o: disc_ranger.h combat.h
$(OBJDIR)disc_shaman.o: disc_shaman.h combat.h
$(OBJDIR)disc_slash.o: disc_slash.h
$(OBJDIR)disc_smythe.o: disc_smythe.h
$(OBJDIR)disc_sorcery.o: disc_sorcery.h combat.h spelltask.h
$(OBJDIR)disc_spirit.o: disc_spirit.h combat.h spelltask.h
$(OBJDIR)disc_survival.o: disc_survival.h combat.h components.h
$(OBJDIR)disc_mindbody.o: disc_mindbody.h combat.h
$(OBJDIR)disc_theology.o: disc_theology.h
$(OBJDIR)disc_thief.o: disc_thief.h $(CASINO_HEADS) combat.h
$(OBJDIR)disc_warrior.o: disc_warrior.h combat.h
$(OBJDIR)disc_water.o: disc_water.h combat.h spelltask.h
$(OBJDIR)disc_wrath.o: disc_wrath.h combat.h
$(OBJDIR)discipline.o: $(DISC_HEADS) components.h combat.h spelltask.h
$(OBJDIR)drawpoker.o: $(CASINO_HEADS)
$(OBJDIR)edit.o: socket.h
$(OBJDIR)food.o: shop.h
$(OBJDIR)gaining.o: statistics.h
$(OBJDIR)gin.o: $(CASINO_HEADS)
$(OBJDIR)handler.o: combat.h range.h connect.h shop.h
$(OBJDIR)hearts.o: $(CASINO_HEADS)
$(OBJDIR)hospital.o: combat.h
$(OBJDIR)immortal.o: statistics.h mail.h $(CASINO_HEADS) combat.h
$(OBJDIR)immortal.o: socket.h systemtask.h disc_shaman_armadillo.h disc_nature.h shop.h
$(OBJDIR)indexdata.o: statistics.h
$(OBJDIR)info.o: statistics.h $(CASINO_HEADS) combat.h components.h
$(OBJDIR)info.o: skillsort.h
$(OBJDIR)inventory.o: statistics.h disc_looting.h $(CASINO_HEADS)
$(OBJDIR)limits.o: statistics.h games.h
$(OBJDIR)loadset.o: loadset.h
$(OBJDIR)magic_skills.o: combat.h
$(OBJDIR)magicutils.o: components.h
$(OBJDIR)mail.o: mail.h
$(OBJDIR)makecorpse.o: combat.h components.h
$(OBJDIR)materials.o: statistics.h
$(OBJDIR)mob_loader.o: components.h
$(OBJDIR)mobact.o: range.h statistics.h
$(OBJDIR)money.o: statistics.h
$(OBJDIR)monster.o: statistics.h
$(OBJDIR)movement.o: combat.h $(CASINO_HEADS) disc_looting.h
$(OBJDIR)obj.o: shop.h
$(OBJDIR)obj_appliedsub.o: create_engine.h
$(OBJDIR)obj_base_clothing.o: shop.h
$(OBJDIR)obj_base_weapon.o: combat.h statistics.h shop.h
$(OBJDIR)obj_bed.o: $(CASINO_HEADS)
$(OBJDIR)obj_board.o: board.h
$(OBJDIR)obj_bow.o: range.h
$(OBJDIR)obj_open_container.o: disc_looting.h
$(OBJDIR)obj_symbol.o: shop.h
$(OBJDIR)offense.o: $(CASINO_HEADS) combat.h
$(OBJDIR)organic.o: shop.h
$(OBJDIR)other.o: $(DISC_HEADS) mail.h range.h $(CASINO_HEADS) skillsort.h
$(OBJDIR)other.o: combat.h statistics.h spelltask.h
$(OBJDIR)parse.o: mail.h
$(OBJDIR)periodic.o: mail.h combat.h components.h disc_meditation.h
$(OBJDIR)physics.o: combat.h
$(OBJDIR)player_data.o: mail.h statistics.h combat.h shop.h
$(OBJDIR)posse.o: paths.h
$(OBJDIR)prof_cl_th.o: profession.h prof_cleric.h prof_thief.h
$(OBJDIR)prof_cl_th.o: prof_cl_th.h
$(OBJDIR)prof_cl_wa.o: profession.h prof_cleric.h prof_warrior.h
$(OBJDIR)prof_cl_wa.o: prof_cl_wa.h
$(OBJDIR)prof_cl_wa_th.o: profession.h prof_cleric.h prof_warrior.h
$(OBJDIR)prof_cl_wa_th.o: prof_thief.h prof_cl_wa_th.h
$(OBJDIR)prof_cleric.o: profession.h prof_cleric.h
$(OBJDIR)prof_deikhan.o: prof_warrior.h prof_deikhan.h
$(OBJDIR)prof_ma_th.o: profession.h prof_mage.h prof_thief.h
$(OBJDIR)prof_ma_th.o: prof_ma_th.h
$(OBJDIR)prof_ma_wa.o: profession.h prof_mage.h prof_warrior.h
$(OBJDIR)prof_ma_wa.o: prof_ma_wa.h
$(OBJDIR)prof_ma_wa_th.o: profession.h prof_mage.h prof_warrior.h
$(OBJDIR)prof_ma_wa_th.o: prof_thief.h prof_ma_wa_th.h
$(OBJDIR)prof_mage.o: prof_mage.h
$(OBJDIR)prof_monk.o: prof_warrior.h prof_monk.h
$(OBJDIR)prof_ranger.o: prof_warrior.h prof_ranger.h
$(OBJDIR)prof_shaman.o: profession.h prof_shaman.h
$(OBJDIR)prof_thief.o: profession.h prof_thief.h
$(OBJDIR)prof_wa_th.o: profession.h prof_warrior.h prof_thief.h
$(OBJDIR)prof_wa_th.o: prof_wa_th.h
$(OBJDIR)prof_warrior.o: profession.h prof_warrior.h
$(OBJDIR)profession.o: profession.h prof_mage.h prof_thief.h prof_cleric.h
$(OBJDIR)profession.o: prof_warrior.h prof_monk.h prof_deikhan.h prof_ranger.h
$(OBJDIR)profession.o: prof_shaman.h prof_cl_th.h prof_cl_wa.h prof_ma_th.h
$(OBJDIR)profession.o: prof_ma_wa.h prof_wa_th.h
$(OBJDIR)quiver.o: disc_looting.h
$(OBJDIR)range.o: combat.h range.h
$(OBJDIR)rent.o: ansi.h statistics.h components.h
$(OBJDIR)repair.o: statistics.h
$(OBJDIR)riding.o: combat.h
$(OBJDIR)shop.o: shop.h statistics.h
$(OBJDIR)show.o: disc_looting.h combat.h components.h
$(OBJDIR)skill_dam.o: statistics.h
$(OBJDIR)skills.o: $(DISC_HEADS)
$(OBJDIR)slots.o: $(CASINO_HEADS)
$(OBJDIR)socket.o: socket.h systemtask.h statistics.h spelltask.h
$(OBJDIR)spec_mobs.o: combat.h components.h paths.h
$(OBJDIR)spec_mobs.o: disc_sorcery.h disc_afflictions.h disc_aegis.h
$(OBJDIR)spec_mobs.o: disc_alchemy.h disc_wrath.h disc_earth.h disc_water.h
$(OBJDIR)spec_rooms.o: statistics.h
$(OBJDIR)spell_parser.o: $(DISC_HEADS)
$(OBJDIR)spelltask.o: $(DISC_HEADS) combat.h spelltask.h
$(OBJDIR)statistics.o: statistics.h systemtask.h
$(OBJDIR)structs.o: combat.h board.h statistics.h $(CASINO_HEADS)
$(OBJDIR)sys_loot.o: sys_loot.h
$(OBJDIR)systemtask.o: systemtask.h
$(OBJDIR)task.o: disc_alchemy.h
$(OBJDIR)task.o: disc_looting.h disc_air.h
$(OBJDIR)task_whittle.o: task_whittle.h
$(OBJDIR)trap.o: disc_looting.h
$(OBJDIR)utility.o: statistics.h shop.h socket.h combat.h $(CASINO_HEADS)

.SUFFIXES:     .cc 
