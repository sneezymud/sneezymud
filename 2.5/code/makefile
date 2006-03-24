##
CC=gcc
##
LD=gcc
##
CFLAGS = -g -DSITELOCK -DNODUPLICATES -DLIMITED_ITEMS
##
##
DIKU = /cis/santa0/jdb/diku
##
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
HEADERFILES = structs.h utils.h comm.h interpreter.h db.h
##
OBJS = comm.o act.comm.o act.informative.o act.move.o act.obj1.o act.obj2.o \
	act.offensive.o act.other.o act.social.o act.wizard.o handler.o \
	db.o interpreter.o utility.o spec_assign.o shop.o limits.o mobact.o \
	fight.o modify.o weather.o spells1.o spells2.o spell_parser.o \
	reception.o constants.o spec_procs.o signals.o board.o magic.o \
	magic2.o skills.o Opinion.o Trap.o magicutils.o multiclass.o hash.o \
	Sound.o Heap.o casino.o oset.o new.o spec_procs2.o games.o mail.o \
	blk_save.o

SRCS = comm.c act.comm.c act.informative.c act.move.c act.obj1.c act.obj2.c \
	act.offensive.c act.other.c act.social.c act.wizard.c handler.c \
	db.c interpreter.c utility.c spec_assign.c shop.c limits.c mobact.c \
	fight.c modify.c weather.c spells1.c spells2.c spell_parser.c \
	reception.c constants.c spec_procs.c signals.c board.c magic.c \
	magic2.c skills.c Opinion.c Trap.c magicutils.c multiclass.c hash.c \
	Sound.c Heap.c casino.c oset.c new.c spec_procs2.c games.c mail.c \
	blk_save.c

all : dmserver.next

dmserver.next : ${OBJS}
## if you want to add debugging stuff, uncomment the following, and comment
##  out the line after that
	$(CC) -o $@ -g ${OBJS} -lcrypt -L/usr/lib/libcrypt.a
#	chown brutius *
##
##depend :
##	makedepend ${SRCS}
clean:
	- /bin/rm *.o > /dev/null 2>&1
install_stuff:
	./NEW/install_stuff




# DO NOT DELETE THIS LINE -- make depend depends on it.

comm.o: /usr/include/netinet/in.h /usr/include/netdb.h /usr/include/string.h
comm.o: utils.h comm.h interpreter.h handler.h db.h
act.comm.o: /usr/include/stdio.h /usr/include/string.h
act.comm.o: handler.h db.h spells.h
act.informative.o: /usr/include/stdio.h /usr/include/string.h
act.informative.o: handler.h db.h spells.h limits.h race.h trap.h hash.h
act.move.o: /usr/include/stdio.h /usr/include/string.h
act.move.o: handler.h db.h spells.h trap.h
act.obj1.o: /usr/include/stdio.h /usr/include/string.h
act.obj1.o: handler.h db.h spells.h trap.h
act.obj2.o: /usr/include/stdio.h /usr/include/string.h
act.obj2.o: comm.h interpreter.h handler.h db.h spells.h
act.offensive.o: /usr/include/stdio.h /usr/include/string.h
act.offensive.o: utils.h comm.h interpreter.h handler.h db.h spells.h
act.offensive.o: limits.h opinion.h race.h multiclass.h
act.other.o: /usr/include/stdio.h /usr/include/string.h
act.other.o: comm.h interpreter.h handler.h db.h spells.h limits.h
act.social.o: /usr/include/stdio.h /usr/include/string.h
act.social.o: handler.h db.h spells.h
act.wizard.o: /usr/include/stdio.h /usr/include/string.h
act.wizard.o: comm.h interpreter.h handler.h db.h spells.h limits.h heap.h
act.wizard.o: hash.h
handler.o: utils.h comm.h db.h handler.h interpreter.h spells.h
db.o: comm.h handler.h limits.h race.h opinion.h hash.h
interpreter.o: /usr/include/ctype.h /usr/include/stdio.h
interpreter.o: limits.h race.h
utility.o: /usr/include/stdio.h /usr/include/malloc.h /usr/include/string.h
utility.o: comm.h hash.h area.h
shop.o: /usr/include/stdio.h /usr/include/string.h
shop.o: utils.h
limits.o: race.h
mobact.o: handler.h db.h comm.h opinion.h trap.h spells.h
fight.o: /usr/include/stdio.h /usr/include/string.h
fight.o: handler.h interpreter.h db.h spells.h
modify.o: /usr/include/stdio.h /usr/include/ctype.h /usr/include/string.h
modify.o: interpreter.h handler.h db.h comm.h
weather.o: /usr/include/stdio.h /usr/include/string.h
weather.o: interpreter.h db.h
spells1.o: comm.h db.h interpreter.h spells.h handler.h s_list.h
spells2.o: comm.h db.h interpreter.h spells.h handler.h poly.h
spell_parser.o: interpreter.h spells.h handler.h
reception.o: handler.h db.h interpreter.h utils.h spells.h
spec_procs.o: /usr/include/stdio.h /usr/include/string.h
spec_procs.o: comm.h interpreter.h handler.h db.h spells.h limits.h s_list.h
spec_procs.o: opinion.h hash.h area.h
spec_procs2.o: structs.h
spec_procs2.o: comm.h interpreter.h handler.h db.h spells.h limits.h s_list.h
spec_procs2.o: opinion.h hash.h area.h utils.h
games.o: structs.h comm.h interpreter.h handler.h spells.h limits.h
games.o: utils.h
board.o: /usr/include/stdio.h /usr/include/ctype.h structs.h
board.o: db.h
magic.o: limits.h db.h
magic2.o: limits.h db.h race.h
skills.o: race.h spells.h comm.h handler.h hash.h
Opinion.o: /usr/include/stdio.h /usr/include/string.h
Opinion.o: db.h
magicutils.o: limits.h
multiclass.o: /usr/include/stdio.h /usr/include/string.h
multiclass.o: handler.h spells.h limits.h opinion.h race.h multiclass.h
Sound.o: /usr/include/stdio.h /usr/include/string.h
Sound.o: db.h spells.h trap.h
Heap.o: comm.h
casino.o: utils.h comm.h interpreter.h handler.h spells.h limits.h 
casino.o: opinion.h structs.h
oset.o: utils.h comm.h interpreter.h handler.h spells.h limits.h
oset.o: opinion.h structs.h
new.o: utils.h comm.h interpreter.h handler.h spells.h limits.h
new.o: opinion.h structs.h
