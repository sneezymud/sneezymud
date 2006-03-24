##########################################################################
##
## SneezyMUD - All rights reserved, SneezyMUD Coding Team
##
##########################################################################

###############################################################################
#                                 Pre-Defines                                 #
###############################################################################
NAME = dmserver.next

###############################################################################
#                                Type-Defines                                 #
###############################################################################
# Compiler Type
# CC = checkerg++
# CC = /usr/pubsw/bin/g++
# GPROFF=1    # Use with care, mud stays up for about 45 minutes with this on.
# USECRYPT=1  # Uncomment to have all the crypt stuff

CC = /usr/bin/gcc
#CC = /usr/local/bin/g++41

###############################################################################
#                    Base Values (Do Not Modify/Add Here)                     #
###############################################################################
CFLAGS = # empty



################# this is from the original 2.5 makefile
##
##  To make some optimizations, you need to add things to the line above.
##  to use hash tables for rooms  (More cpu, less storage), add
##  -DHASH    
##
##  To get rid of tracking (lower cpu)
##  -DNOTRACK
##
##  To limit items above a specific rent cost
##  -DLIMITED_ITEMS
##
##  To force new players to be authorized
##  -DPLAYER_AUTH
##
##  To add some new debugging stuff (mostly limited to malloc_debug)
##  -DDEBUG
##  To add some commands for locking out specific hosts
##  -DSITELOCK
##
##  To modify some commands to make item duplication near impossible:
##  -DNODUPLICATES
##
#################################




###############################################################################
#                                   CFLAGS                                    #
###############################################################################
# CFLAGS +=-static      # link statically
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
CFLAGS +=-O2            # optimize code
OBJDIR = objs/


#CFLAGS +=-O3           # optimize code
#CFLAGS +=-fhandle-exceptions
#CFLAGS +=-frtti
##CFLAGS +=-Wall                  # all rec. warnings shown (includes all above)
##CFLAGS +=-Wpointer-arith        # looks for things that depend on sizeof()
##CFLAGS +=-Wcast-qual            # Warns about using casting to remove const
##CFLAGS +=-Wcast-align           # a pointer cast changes alignment
##CFLAGS +=-Wwrite-strings        # looks for formal const char* sent to char*
##CFLAGS +=-Wconversion           # bad conversions
#CFLAGS +=-Wstrict-prototypes    # functions declared sans argument types
#CFLAGS +=-Wmissing-prototypes   # global functions without a prototype
#CFLAGS +=-Wmissing-declarations # global function without a declaration
#CFLAGS +=-Wnested-externs       # extern found within a func
#CFLAGS +=-Woverloaded-virtual   # virtual without a declaration in base class
##CFLAGS +=-Werror                # turns a warning into an error

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

# for dynamic libraries
#CFLAGS +=-fPIC

CFLAGS += -I/usr/local/include

###############################################################################
#                                   LDFLAGS                                   #
###############################################################################
LDFLAGS=-g # enable core debugging
#LDFLAGS +=-L/usr/lib # specify this directory to look in
# LDFLAGS +=-L/usr/pubsw/lib # specify this directory to look in
# **** libg++ and libm seem to be equivalent for Gnu 2.7.2 and before
# **** The standard C++ libraries
# LDFLAGS +=-Bdynamic -lg++
# LDFLAGS +=-bstatic -lg++
# LDFLAGS +=-lg++
# LDFLAGS +=/usr/pubsw/lib/libg++.a # enable for insight -j
# LDFLAGS +=/usr/lib/libg++.a # enable for insight -j
#LDFLAGS +=-lstdc++
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
#LDFLAGS +=/usr/pubsw/lib/libcrypt.a # enable for insight
else
LDFLAGS +=/usr/lib/libcrypt.a # enable for insight
endif
ifdef GPROFF
LDFLAGS +=-pg #              gprof mapping
endif
LDFLAGS += -L/usr/local/lib
#LDFLAGS +=-fhandle-exceptions
# postgresql database lib
#LDFLAGS +=-lpq++
#LDFLAGS +=-lpq -lares

###############################################################################
##                        Actual Compiling Starts Here                       ##
###############################################################################


# this is a list of objects to be compiled and linked
# just add new ones to the bottom
OBJS = comm.o act.comm.o act.informative.o act.move.o act.obj1.o act.obj2.o \
	act.offensive.o act.other.o act.social.o act.wizard.o handler.o \
	db.o interpreter.o utility.o spec_assign.o shop.o limits.o mobact.o \
	fight.o modify.o weather.o spells1.o spells2.o spell_parser.o \
	reception.o constants.o spec_procs.o signals.o board.o magic.o \
	magic2.o skills.o Opinion.o Trap.o magicutils.o multiclass.o hash.o \
	Sound.o Heap.o casino.o oset.o new.o spec_procs2.o games.o mail.o \
	blk_save.o

OBJS := $(sort $(addprefix $(OBJDIR), $(OBJS)))

all: exe

exe: $(NAME)

# normal executable
$(NAME) : $(OBJS)
	@echo \*\*\* Making Final Link: $(NAME)
	@$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)
	@echo \*\*\* Make Process Complete
	@echo "`date`"


clean:
	/bin/rm -f $(OBJDIR)*.o

depend:
	makedepend -I/usr/local/include -I/usr/include/c++/3.4/ -p$(OBJDIR) -- $(CFLAGS) -- *.c


$(OBJDIR)%.o : %.c
	@echo Making \"$<\"
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.c
	@echo Making \"$<\"
	$(CC) $(CFLAGS) -c $< -o $(OBJDIR)$@

.SUFFIXES:     .c 

###############################################################################
#                    Dependencies
###############################################################################
# Everything under here is generated by "make depend" so don't modify it

# DO NOT DELETE

objs/Heap.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/Heap.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/Heap.o: /usr/include/machine/_types.h structs.h /usr/include/sys/types.h
objs/Heap.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/Heap.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/Heap.o: /usr/include/sys/timespec.h heap.h comm.h
objs/Opinion.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/Opinion.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/Opinion.o: /usr/include/machine/_types.h /usr/include/string.h
objs/Opinion.o: /usr/include/strings.h /usr/include/ctype.h
objs/Opinion.o: /usr/include/_ctype.h /usr/include/time.h
objs/Opinion.o: /usr/include/sys/timespec.h structs.h
objs/Opinion.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/Opinion.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/Opinion.o: /usr/include/sys/_timeval.h utils.h spells.h race.h opinion.h
objs/Opinion.o: db.h
objs/Sound.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/Sound.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/Sound.o: /usr/include/machine/_types.h /usr/include/string.h
objs/Sound.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/Sound.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/Sound.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/Sound.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/Sound.o: handler.h db.h spells.h trap.h
objs/Trap.o: structs.h /usr/include/sys/types.h /usr/include/sys/cdefs.h
objs/Trap.o: /usr/include/machine/endian.h /usr/include/sys/_types.h
objs/Trap.o: /usr/include/machine/_types.h /usr/include/sys/select.h
objs/Trap.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/Trap.o: /usr/include/sys/timespec.h utils.h spells.h trap.h comm.h
objs/act.comm.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.comm.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.comm.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.comm.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/act.comm.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/act.comm.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/act.comm.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/act.comm.o: handler.h db.h spells.h
objs/act.informative.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.informative.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.informative.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.informative.o: /usr/include/strings.h /usr/include/ctype.h
objs/act.informative.o: /usr/include/_ctype.h /usr/include/time.h
objs/act.informative.o: /usr/include/sys/timespec.h structs.h
objs/act.informative.o: /usr/include/sys/types.h
objs/act.informative.o: /usr/include/machine/endian.h
objs/act.informative.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/act.informative.o: /usr/include/sys/_timeval.h utils.h comm.h
objs/act.informative.o: interpreter.h handler.h db.h spells.h limits.h race.h
objs/act.informative.o: trap.h hash.h
objs/act.move.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.move.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.move.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.move.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/act.move.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/act.move.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/act.move.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/act.move.o: handler.h db.h spells.h trap.h
objs/act.obj1.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.obj1.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.obj1.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.obj1.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/act.obj1.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/act.obj1.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/act.obj1.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/act.obj1.o: handler.h db.h spells.h trap.h
objs/act.obj2.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.obj2.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.obj2.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.obj2.o: /usr/include/strings.h /usr/include/ctype.h
objs/act.obj2.o: /usr/include/_ctype.h /usr/include/assert.h structs.h
objs/act.obj2.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/act.obj2.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/act.obj2.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/act.obj2.o: utils.h comm.h interpreter.h handler.h db.h spells.h
objs/act.offensive.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.offensive.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.offensive.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.offensive.o: /usr/include/strings.h structs.h
objs/act.offensive.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/act.offensive.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/act.offensive.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/act.offensive.o: utils.h comm.h interpreter.h handler.h db.h spells.h
objs/act.offensive.o: limits.h opinion.h race.h multiclass.h
objs/act.other.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.other.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.other.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.other.o: /usr/include/strings.h /usr/include/ctype.h
objs/act.other.o: /usr/include/_ctype.h structs.h /usr/include/sys/types.h
objs/act.other.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/act.other.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/act.other.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/act.other.o: handler.h db.h spells.h limits.h
objs/act.social.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.social.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.social.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.social.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/act.social.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/act.social.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/act.social.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/act.social.o: handler.h db.h spells.h
objs/act.wizard.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/act.wizard.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/act.wizard.o: /usr/include/machine/_types.h /usr/include/string.h
objs/act.wizard.o: /usr/include/strings.h /usr/include/ctype.h
objs/act.wizard.o: /usr/include/_ctype.h /usr/include/unistd.h
objs/act.wizard.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/act.wizard.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/act.wizard.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/act.wizard.o: /usr/include/sys/unistd.h structs.h utils.h comm.h
objs/act.wizard.o: interpreter.h handler.h db.h spells.h limits.h heap.h
objs/act.wizard.o: hash.h race.h
objs/blk_save.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/blk_save.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/blk_save.o: /usr/include/machine/_types.h /usr/include/sys/time.h
objs/blk_save.o: /usr/include/sys/_timeval.h /usr/include/sys/types.h
objs/blk_save.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/blk_save.o: /usr/include/sys/_sigset.h /usr/include/sys/timespec.h
objs/blk_save.o: /usr/include/time.h structs.h comm.h handler.h db.h
objs/blk_save.o: interpreter.h utils.h spells.h limits.h heap.h hash.h race.h
objs/board.o: /usr/include/string.h /usr/include/sys/cdefs.h
objs/board.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/board.o: /usr/include/machine/_types.h /usr/include/strings.h
objs/board.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/_ctype.h
objs/board.o: /usr/include/time.h /usr/include/sys/timespec.h structs.h
objs/board.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/board.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/board.o: /usr/include/sys/_timeval.h utils.h comm.h db.h
objs/casino.o: /usr/include/string.h /usr/include/sys/cdefs.h
objs/casino.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/casino.o: /usr/include/machine/_types.h /usr/include/strings.h structs.h
objs/casino.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/casino.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/casino.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/casino.o: spells.h utils.h
objs/class.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/class.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/class.o: /usr/include/machine/_types.h /usr/include/string.h
objs/class.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/class.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/class.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/class.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/class.o: handler.h spells.h limits.h opinion.h race.h
objs/comm.o: /usr/include/errno.h /usr/include/sys/cdefs.h
objs/comm.o: /usr/include/stdio.h /usr/include/sys/_null.h
objs/comm.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
objs/comm.o: /usr/include/ctype.h /usr/include/_ctype.h
objs/comm.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/comm.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/comm.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/comm.o: /usr/include/sys/socket.h /usr/include/sys/_iovec.h
objs/comm.o: /usr/include/machine/param.h /usr/include/sys/wait.h
objs/comm.o: /usr/include/netinet/in.h /usr/include/netinet6/in6.h
objs/comm.o: /usr/include/netdb.h /usr/include/string.h
objs/comm.o: /usr/include/strings.h /usr/include/sys/time.h
objs/comm.o: /usr/include/time.h /usr/include/fcntl.h /usr/include/signal.h
objs/comm.o: /usr/include/sys/signal.h /usr/include/machine/signal.h
objs/comm.o: /usr/include/machine/trap.h /usr/include/sys/resource.h
objs/comm.o: structs.h utils.h comm.h interpreter.h handler.h db.h
objs/constants.o: structs.h /usr/include/sys/types.h /usr/include/sys/cdefs.h
objs/constants.o: /usr/include/machine/endian.h /usr/include/sys/_types.h
objs/constants.o: /usr/include/machine/_types.h /usr/include/sys/select.h
objs/constants.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/constants.o: /usr/include/sys/timespec.h limits.h trap.h
objs/craps.o: /usr/include/string.h /usr/include/sys/cdefs.h
objs/craps.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/craps.o: /usr/include/machine/_types.h /usr/include/strings.h structs.h
objs/craps.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/craps.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/craps.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/craps.o: spells.h utils.h
objs/db.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/db.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/db.o: /usr/include/machine/_types.h /usr/include/string.h
objs/db.o: /usr/include/strings.h /usr/include/ctype.h /usr/include/_ctype.h
objs/db.o: /usr/include/time.h /usr/include/sys/timespec.h structs.h
objs/db.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/db.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/db.o: /usr/include/sys/_timeval.h utils.h db.h comm.h handler.h limits.h
objs/db.o: race.h mail.h opinion.h hash.h
objs/fight.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/fight.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/fight.o: /usr/include/machine/_types.h /usr/include/string.h
objs/fight.o: /usr/include/strings.h /usr/include/assert.h structs.h
objs/fight.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/fight.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/fight.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h race.h
objs/fight.o: utils.h comm.h handler.h interpreter.h db.h spells.h
objs/games.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/games.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/games.o: /usr/include/machine/_types.h /usr/include/string.h
objs/games.o: /usr/include/strings.h /usr/include/ctype.h
objs/games.o: /usr/include/_ctype.h /usr/include/time.h
objs/games.o: /usr/include/sys/timespec.h structs.h /usr/include/sys/types.h
objs/games.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/games.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h games.h
objs/games.o: utils.h comm.h interpreter.h handler.h db.h spells.h limits.h
objs/games.o: race.h trap.h hash.h
objs/handler.o: /usr/include/string.h /usr/include/sys/cdefs.h
objs/handler.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/handler.o: /usr/include/machine/_types.h /usr/include/strings.h
objs/handler.o: /usr/include/stdio.h /usr/include/ctype.h
objs/handler.o: /usr/include/_ctype.h /usr/include/assert.h structs.h
objs/handler.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/handler.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/handler.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/handler.o: utils.h comm.h db.h handler.h interpreter.h spells.h
objs/hash.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/hash.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/hash.o: /usr/include/machine/_types.h structs.h /usr/include/sys/types.h
objs/hash.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/hash.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/hash.o: /usr/include/sys/timespec.h hash.h
objs/interpreter.o: /usr/include/dirent.h /usr/include/sys/cdefs.h
objs/interpreter.o: /usr/include/sys/dirent.h /usr/include/sys/_types.h
objs/interpreter.o: /usr/include/machine/_types.h /usr/include/sys/_null.h
objs/interpreter.o: /usr/include/unistd.h /usr/include/sys/types.h
objs/interpreter.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/interpreter.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/interpreter.o: /usr/include/sys/timespec.h /usr/include/sys/unistd.h
objs/interpreter.o: /usr/include/netdb.h /usr/include/netinet/in.h
objs/interpreter.o: /usr/include/netinet6/in6.h /usr/include/sys/param.h
objs/interpreter.o: /usr/include/sys/syslimits.h /usr/include/sys/signal.h
objs/interpreter.o: /usr/include/machine/signal.h /usr/include/machine/trap.h
objs/interpreter.o: /usr/include/machine/param.h /usr/include/sys/limits.h
objs/interpreter.o: /usr/include/machine/_limits.h /usr/include/sys/syscall.h
objs/interpreter.o: /usr/include/string.h /usr/include/strings.h
objs/interpreter.o: /usr/include/ctype.h /usr/include/_ctype.h
objs/interpreter.o: /usr/include/stdio.h /usr/include/arpa/telnet.h structs.h
objs/interpreter.o: comm.h interpreter.h db.h utils.h limits.h mail.h race.h
objs/interpreter.o: spells.h
objs/limits.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/limits.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/limits.o: /usr/include/machine/_types.h /usr/include/assert.h structs.h
objs/limits.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/limits.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/limits.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/limits.o: limits.h utils.h spells.h comm.h race.h
objs/magic.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/magic.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/magic.o: /usr/include/machine/_types.h /usr/include/assert.h structs.h
objs/magic.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/magic.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/magic.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h utils.h
objs/magic.o: comm.h spells.h handler.h limits.h db.h race.h
objs/magic2.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/magic2.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/magic2.o: /usr/include/machine/_types.h /usr/include/assert.h structs.h
objs/magic2.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/magic2.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/magic2.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/magic2.o: utils.h comm.h spells.h handler.h limits.h db.h race.h
objs/magicutils.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/magicutils.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/magicutils.o: /usr/include/machine/_types.h /usr/include/assert.h
objs/magicutils.o: structs.h /usr/include/sys/types.h
objs/magicutils.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/magicutils.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/magicutils.o: /usr/include/sys/timespec.h utils.h comm.h spells.h
objs/magicutils.o: handler.h limits.h
objs/mail.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/mail.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/mail.o: /usr/include/machine/_types.h /usr/include/assert.h
objs/mail.o: /usr/include/ctype.h /usr/include/_ctype.h /usr/include/string.h
objs/mail.o: /usr/include/strings.h /usr/include/stdlib.h /usr/include/time.h
objs/mail.o: /usr/include/sys/timespec.h mail.h structs.h
objs/mail.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/mail.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/mail.o: /usr/include/sys/_timeval.h utils.h comm.h db.h
objs/mobact.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/mobact.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/mobact.o: /usr/include/machine/_types.h structs.h
objs/mobact.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/mobact.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/mobact.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/mobact.o: utils.h handler.h db.h comm.h opinion.h trap.h spells.h
objs/modify.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/modify.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/modify.o: /usr/include/machine/_types.h /usr/include/ctype.h
objs/modify.o: /usr/include/_ctype.h /usr/include/string.h
objs/modify.o: /usr/include/strings.h /usr/include/time.h
objs/modify.o: /usr/include/sys/timespec.h structs.h /usr/include/sys/types.h
objs/modify.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/modify.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h utils.h
objs/modify.o: interpreter.h handler.h db.h comm.h
objs/multiclass.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/multiclass.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/multiclass.o: /usr/include/machine/_types.h /usr/include/string.h
objs/multiclass.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/multiclass.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/multiclass.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/multiclass.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/multiclass.o: handler.h spells.h limits.h opinion.h race.h multiclass.h
objs/new.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/new.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/new.o: /usr/include/machine/_types.h /usr/include/assert.h structs.h
objs/new.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/new.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/new.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h utils.h
objs/new.o: comm.h spells.h handler.h limits.h db.h race.h
objs/oset.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/oset.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/oset.o: /usr/include/machine/_types.h /usr/include/ctype.h
objs/oset.o: /usr/include/_ctype.h /usr/include/string.h
objs/oset.o: /usr/include/strings.h /usr/include/time.h
objs/oset.o: /usr/include/sys/timespec.h structs.h /usr/include/sys/types.h
objs/oset.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/oset.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h utils.h
objs/oset.o: interpreter.h handler.h db.h comm.h spells.h limits.h opinion.h
objs/reception.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/reception.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/reception.o: /usr/include/machine/_types.h /usr/include/sys/time.h
objs/reception.o: /usr/include/sys/_timeval.h /usr/include/sys/types.h
objs/reception.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/reception.o: /usr/include/sys/_sigset.h /usr/include/sys/timespec.h
objs/reception.o: /usr/include/time.h structs.h comm.h handler.h db.h
objs/reception.o: interpreter.h utils.h spells.h
objs/shop.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/shop.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/shop.o: /usr/include/machine/_types.h /usr/include/string.h
objs/shop.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/shop.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/shop.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/shop.o: /usr/include/sys/timespec.h comm.h handler.h db.h interpreter.h
objs/shop.o: utils.h
objs/signals.o: /usr/include/signal.h /usr/include/sys/cdefs.h
objs/signals.o: /usr/include/sys/_types.h /usr/include/machine/_types.h
objs/signals.o: /usr/include/sys/signal.h /usr/include/sys/_sigset.h
objs/signals.o: /usr/include/machine/signal.h /usr/include/machine/trap.h
objs/signals.o: /usr/include/stdio.h /usr/include/sys/_null.h
objs/signals.o: /usr/include/sys/time.h /usr/include/sys/_timeval.h
objs/signals.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/signals.o: /usr/include/sys/select.h /usr/include/sys/timespec.h
objs/signals.o: /usr/include/time.h structs.h utils.h
objs/skills.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/skills.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/skills.o: /usr/include/machine/_types.h structs.h
objs/skills.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/skills.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/skills.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/skills.o: utils.h race.h spells.h comm.h handler.h hash.h trap.h
objs/spec_assign.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/spec_assign.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/spec_assign.o: /usr/include/machine/_types.h structs.h
objs/spec_assign.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/spec_assign.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/spec_assign.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/spec_assign.o: db.h utils.h
objs/spec_procs.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/spec_procs.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/spec_procs.o: /usr/include/machine/_types.h /usr/include/string.h
objs/spec_procs.o: /usr/include/strings.h /usr/include/ctype.h
objs/spec_procs.o: /usr/include/_ctype.h structs.h /usr/include/sys/types.h
objs/spec_procs.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/spec_procs.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/spec_procs.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/spec_procs.o: handler.h db.h spells.h limits.h s_list.h opinion.h hash.h
objs/spec_procs.o: area.h race.h
objs/spec_procs2.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/spec_procs2.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/spec_procs2.o: /usr/include/machine/_types.h /usr/include/string.h
objs/spec_procs2.o: /usr/include/strings.h /usr/include/ctype.h
objs/spec_procs2.o: /usr/include/_ctype.h structs.h /usr/include/sys/types.h
objs/spec_procs2.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/spec_procs2.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/spec_procs2.o: /usr/include/sys/timespec.h utils.h comm.h interpreter.h
objs/spec_procs2.o: handler.h db.h spells.h limits.h s_list.h opinion.h
objs/spec_procs2.o: hash.h area.h race.h
objs/spell_parser.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/spell_parser.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/spell_parser.o: /usr/include/machine/_types.h /usr/include/assert.h
objs/spell_parser.o: structs.h /usr/include/sys/types.h
objs/spell_parser.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/spell_parser.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/spell_parser.o: /usr/include/sys/timespec.h utils.h comm.h db.h
objs/spell_parser.o: interpreter.h spells.h handler.h
objs/spells1.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/spells1.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/spells1.o: /usr/include/machine/_types.h structs.h
objs/spells1.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/spells1.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/spells1.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/spells1.o: utils.h comm.h db.h interpreter.h spells.h handler.h s_list.h
objs/spells2.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/spells2.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/spells2.o: /usr/include/machine/_types.h /usr/include/assert.h structs.h
objs/spells2.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/spells2.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/spells2.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/spells2.o: utils.h comm.h db.h interpreter.h spells.h handler.h poly.h
objs/utility.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/utility.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/utility.o: /usr/include/machine/_types.h /usr/include/stdlib.h
objs/utility.o: /usr/include/string.h /usr/include/strings.h
objs/utility.o: /usr/include/assert.h /usr/include/ctype.h
objs/utility.o: /usr/include/_ctype.h /usr/include/stdarg.h structs.h
objs/utility.o: /usr/include/sys/types.h /usr/include/machine/endian.h
objs/utility.o: /usr/include/sys/select.h /usr/include/sys/_sigset.h
objs/utility.o: /usr/include/sys/_timeval.h /usr/include/sys/timespec.h
objs/utility.o: /usr/include/time.h utils.h spells.h race.h db.h opinion.h
objs/utility.o: comm.h hash.h area.h
objs/weather.o: /usr/include/stdio.h /usr/include/sys/cdefs.h
objs/weather.o: /usr/include/sys/_null.h /usr/include/sys/_types.h
objs/weather.o: /usr/include/machine/_types.h /usr/include/string.h
objs/weather.o: /usr/include/strings.h structs.h /usr/include/sys/types.h
objs/weather.o: /usr/include/machine/endian.h /usr/include/sys/select.h
objs/weather.o: /usr/include/sys/_sigset.h /usr/include/sys/_timeval.h
objs/weather.o: /usr/include/sys/timespec.h utils.h comm.h handler.h
objs/weather.o: interpreter.h db.h
