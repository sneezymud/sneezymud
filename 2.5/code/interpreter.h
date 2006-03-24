/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

void command_interpreter(struct char_data *ch, char *argument);
int search_block(char *arg, char **list, bool exact);
int old_search_block(char *argument,int begin,int length,
		     char **list,int mode);
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
char *one_argument(char *argument,char *first_arg);
void only_argument(char *argument,char *first_arg);
int fill_word(char *argument);
void half_chop(char *string, char *arg1, char *arg2);
void nanny(struct descriptor_data *d, char *arg);
int is_abbrev(char *arg1, char *arg2);


struct command_info
{
	void (*command_pointer) (struct char_data *ch, char *argument, int cmd);
	byte minimum_position;
	byte minimum_level;
};

#define MENU         \
"\n\rWelcome to SneezyMUD\n\r\n\
0) Leave SneezyMUD.\n\r\
1) Enter the game at Midgaard\n\r\
2) Enter description.\n\r\
3) Read the background story\n\r\
4) Change password.\n\r\
5) Enter somewhere else\n\r\
6) Delete this character\n\r\n\r\
   Make your choice: "


#define CLASSHELP        \
"\n\rClasses: (Thief,Cleric,Mage,Warrior,Antipaladin,Paladin,Ranger,Monk)\n\r\
Advantages of being single classed (one class only):\n\r\n\r\
1) Skills for Single Class people can be more specialized.\n\r\
2) Some very powerful spells are available that aren't to others.\n\r\
3) Single classed people gain experience faster than multi-classes\n\r\
Diadvantages of being single Classes:\n\r\n\r\
1) Lack of the ability to go alone(this isnt really a bad thing)\n\r\
2) Fewer hit points than multi-classed people.\n\r\
3) The lack of ability to wear certain items.\n\r\
Advantages for multi-classed people:\n\r\n\r\
1) More hit points than single classed people.\n\r\
2) Ability to go alone(This can be bad at times)\n\r\
3) Ability to wear more items.\n\r\
Disadvantages for multi-classing:\n\r\n\r\
1) Takes longer to gain experience.\n\r\
2) Cant be totally specialized in most skills.\n\r"

#define  RACEHELP          \
"\n\rRaces:  (Dwarven, Elven, Human, Hobbit, Ogre, Gnome)\n\r\
Dwarves:  Shorter. Less Movement. More sturdy. Less wise. Infravision\n\r\
Elves:    Taller.  More Movement. Less sturdy. More dextrous.\n\r\
Humans:   Average...  \n\r\
Hobbits:  Shorter. Least movement of all races. More dextrous. Weaker.\n\r\
Ogres:    Largest of all races.Most movement of all races. Stronger. Dumber\n\r\
Gnomes:   Shorter. Less movement. More intelligent. Less wise.\n\r" 

#define OLDONE \
"\n\r           Tom Madsen, Michael Seifert, and Sebastian Hammer\n\r\
                  Hans Henrik Staerfeldt, Katja Nyboe,\n\r\
                              Created by\n\r\n\r\
                           DikuMUD I (GAMMA 0.0)\n\r\n\r"


#define GREETINGS \
"\n\r                                 SneezyMUD 2.5\n\r\n\r\
                       Original DikuMUD concept created by:\n\r\
                Tom Madsen, Michael Seifert, and Sebastian Hammer\n\r\
                       Hans Henrik Staerfeldt, Katja Nyboe,\n\r\
             Original SneezyMUD code source provided by J. Brothers.\n\r\n\r\
                        Coding done by Brutius, Stargazer\n\r\
                             Lord of Worlds : Batopr\n\r\n\r"


#define WELC_MESSG \
"\n\rWelcome to the land of SneezyMUD. May your visit here be... interesting.\
\n\r\n\r"


#define STORY     \
" SneezyMUD is a creation of the Public broadcasting System. \n \
  Brought to you today by the letters X and B, and the number 69\n\n\r"

