/* Oset.c is Copyright (C) 1992 by Dan Brumleve.  Ignorance or removal  *
 * of this frienndly reminder is punishable by death by slooow torture  */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "spells.h"
#include "limits.h"
#include "opinion.h"


struct oset_field_data {
    char *set[4];
    char *description[4];
};


struct oset_field_data oset_field[25] = {
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"duration", "", "", ""},
      {"(duration of light.  -1 for permanent)", "", "", ""} },
    { {"level", "spell1", "spell2", "spell3"},
      {"(level of spells)", "(first spell)", "(second)", "(third)"} },
    { {"level", "max-charges", "charges", "spell"},
      {"(level of spell)", "(maximum charges)", "(charges left)", "(spell)"} },
    { {"level", "max-charges", "charges", "spell"},
      {"(level of spell)", "(maximum charges)", "(charges left)", "(spell)"} },
    { {"damage", "wtype", "", ""},
      {"(weapon damage, in the form: XdY)", "(weapon type)", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"points", "", "", ""},
      {"(armor points)", "", "", ""} },
    { {"level", "spell1", "spell2", "spell3"},
      {"(level of spells)", "(first spell)", "(second)", "(third)"} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"capacity", "flags", "key", ""},
      {"(container's capacity)", "(container flags)", "(opening key)", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"capacity", "amount", "liquid", "poisoned"},
      {"(capacity)", "(amount left)", "(liqid)", "(poisoned?)"} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"amount", "poisoned", "", ""},
      {"(number of hours restored)", "(poisoned?)", "", ""} },
    { {"amount", "", "", ""},
      {"(amount of money)", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
    { {"", "", "", ""},
      {"", "", "", ""} },
};
    

void set_oedesc(struct char_data *ch, struct obj_data *obj, char *keywds) 
{
    struct extra_descr_data *tmp, *newdesc;
    char buf[256];

    if (!*keywds) {
	send_to_char("You must give a list of keywords.\n\r", ch);
	return;
    }
    for (tmp = obj->ex_description; tmp; tmp = tmp->next) {
	if (!strcasecmp(tmp->keyword, keywds)) {
	    break;
	}
    }

    CREATE(newdesc, struct extra_descr_data, 1);
    newdesc->description = (char *)0;

    if (!tmp) {
	newdesc->next = obj->ex_description;
	obj->ex_description = newdesc;
	newdesc->keyword = strdup(keywds);
    } else {
	send_to_char("The old description was: \n\r\n\r", ch);
	send_to_char(tmp->description, ch);
	free((char *)newdesc);
	newdesc = tmp;
    }

    if(newdesc->description) {
	free((char *)newdesc->description);
	newdesc->description = (char *)0;
    }

    send_to_char("Enter a new description.  Terminate with a '~'\n\r", ch);
    ch->desc->str = &newdesc->description;
    newdesc->description = 0;
    ch->desc->max_str = 1000;
}

void set_mtype(struct char_data *ch, struct obj_data *obj, char *arg) 
{
    char buf1[256], buf2[256], type[100];
    int i;

    static char *obj_type[]=
    {
        "",
	"finger",
	"neck",
	"body",
	"head",
	"legs",
	"feet",
	"hands",
	"arms",
	"shield",
	"about",
	"waiste",
	"wrist",
	"wield",
	"hold",
	"",
	"\n"
    };

    if (!*arg) {
        send_to_char("The following types are available:\n\r\n\r", ch);
        for (i = 0; *obj_type[i] != '\n'; i++) {
            send_to_char(obj_type[i], ch);
            send_to_char("\n\r", ch);
        }
        return;
    }

    one_argument(arg, type);

    if (is_abbrev(type, "finger"))  {
      obj->obj_flags.wear_flags = 3;
    } else if (is_abbrev(type, "neck")) {
      obj->obj_flags.wear_flags = 5;
 } else if (is_abbrev(type, "body")) {
      obj->obj_flags.wear_flags = 9;
 } else if (is_abbrev(type, "head")) {
      obj->obj_flags.wear_flags = 17;
 } else if (is_abbrev(type, "legs")) {
      obj->obj_flags.wear_flags = 33;
 } else if (is_abbrev(type, "feet")) {
      obj->obj_flags.wear_flags = 65;
 } else if (is_abbrev(type, "hands")) {
      obj->obj_flags.wear_flags = 129;
 } else if (is_abbrev(type, "arms")) {
      obj->obj_flags.wear_flags = 257;
 } else if (is_abbrev(type, "shield")) {
      obj->obj_flags.wear_flags = 513;
 } else if (is_abbrev(type, "about")) {
      obj->obj_flags.wear_flags = 1025;
 } else if (is_abbrev(type, "waist")) {
      obj->obj_flags.wear_flags = 2049;
 } else if (is_abbrev(type, "wrist")) {
      obj->obj_flags.wear_flags = 4097;
 } else if (is_abbrev(type, "wield")) {
      obj->obj_flags.wear_flags = 8193;
 } else if (is_abbrev(type, "hold")) {
      obj->obj_flags.wear_flags = 16385;
 } else {
   send_to_char("That is not a valid place for the item to be worn!\n\r", ch);
   return;
 }

    for (i = 0; i < 4; i++)
	obj->obj_flags.value[i] = 0;

    send_to_char("Done.\n\r", ch);
}

void set_oflags(struct char_data *ch, struct obj_data *obj, char *arg) 
{
    char buf1[256], buf2[256], num[100], type[100];
    int i, number;

    static char *obj_type[]=
    {
    "ITEM_GLOW            1",
    "ITEM_HUM             2",
    "ITEM_METAL           4  /* undefined...  */",
    "ITEM_MINERAL         8  /* undefined?    */",
    "ITEM_ORGANIC        16  /* undefined?    */",
    "ITEM_INVISIBLE      32",
    "ITEM_MAGIC          64",
    "ITEM_NODROP        128",
    "ITEM_BLESS         256",
    "ITEM_ANTI_GOOD     512 /* not usable by good people    */",
    "ITEM_ANTI_EVIL    1024 /* not usable by evil people    */",
    "ITEM_ANTI_NEUTRAL 2048 /* not usable by neutral people */",
    "ITEM_ANTI_CLERIC  4096",
    "ITEM_ANTI_MAGE    8192",
    "ITEM_ANTI_THIEF   16384",
    "ITEM_ANTI_FIGHTER 32768",
    "ITEM_BRITTLE      65536 /* weapons that break after 1 hit */",
    "ITEM_LEVEL10      131072 /*cant be worn by levels < 10 */",
    "ITEM_LEVEL20      262144 /*cant be worn by levels < 20 */",
    "ITEM_LEVEL30      524288 /*Cant be worn by levels < 30 */",
		"\n"
    };

    if (!*arg) {
        send_to_char("The following types are available:\n\r\n\r", ch);
        for (i = 0; *obj_type[i] != '\n'; i++) {
            send_to_char(obj_type[i], ch);
            send_to_char("\n\r", ch);
        }
        return;
    send_to_char("Add up the numbers of the flags you want.\n\r", ch);
    send_to_char("For example : an item that is anti_mage and glowing.\n\r", ch);
    send_to_char("Would be 1+8192 or 8193\n\r", ch);
    }

    only_argument(arg, num);
    if(isdigit(*num))
      number = atoi(num);
    else {
      send_to_char("You must enter a number for this flag.\n\r", ch);
      return;
    }
    
    obj->obj_flags.extra_flags = number;

 
    send_to_char("Done.\n\r", ch);
}


void set_otype(struct char_data *ch, struct obj_data *obj, char *arg) 
{
    char buf1[256], buf2[256];
    int type, i;

    static char *obj_type[]=
    {
	"light",
	"scroll",
	"wand",
	"staff",
	"weapon",
	"fireweapon",
	"missile",
	"treasure",
	"armor",
	"potion",
	"worn",
	"other",
	"trash",
	"trap",
	"container",
	"note",
	"drinkcon",
	"key",
	"food",
	"money",
	"pen",
	"boat",
	"\n"
    };

    if (!*arg) {
        send_to_char("The following types are available:\n\r\n\r", ch);
        for (i = 0; *obj_type[i] != '\n'; i++) {
            send_to_char(obj_type[i], ch);
            send_to_char("\n\r", ch);
        }
        return;
    }

    type = old_search_block(arg, 0, strlen(arg), obj_type, 0);
    if (type == -1) {
	send_to_char("That obj type does not exist.\n\r", ch);
	return;
    }

    obj->obj_flags.type_flag = type;

    for (i = 0; i < 4; i++)
	obj->obj_flags.value[i] = 0;

    send_to_char("Done.\n\r", ch);
}


void set_oaffect(struct char_data *ch, struct obj_data *obj, char *arg, int a)
{
    char buf1[256], buf2[256];
    int type, mod, i;

    static char *oaffects[]=
    {
	"strength",
	"dexterity",
	"intelligence",
	"wisdom",
	"constitution",
	"sex",
	"class",
	"level",
	"age",
	"weight",
	"height",
	"mana points",
	"hit points",
	"movement_points",
	"gold",
	"exp",
	"ac",
	"hitroll",
	"damage",
	"saving_para",
	"saving_rod",
	"saving_petri",
	"saving_breath",
	"saving_spell",
	"save all",
        "immune",
        "susceptibility",
        "resistance",
        "spell_affects",
        "weapon_spells",
        "eat spells",
        "backstab",
        "kick",
        "sneak",
        "hide",
        "bash",
        "pick",
        "steal",
        "track",
        "hitndam",
	"\n"
    };

    if (!*arg) {
        send_to_char("The following affects are available:\n\r\n\r", ch);
        send_to_char("none\n\r", ch);
        for (i = 0; *oaffects[i] != '\n'; i++) {
            send_to_char(oaffects[i], ch);
            send_to_char("\n\r", ch);
        }
        return;
    }

    if (!strcasecmp(arg, "none")) {
        obj->affected[a].location = 0;
        obj->affected[a].modifier = 0;
        send_to_char("Done.\n\r", ch);
        return;
    }

    arg = one_argument(arg, buf1);
    arg = one_argument(arg, buf2);

    type = old_search_block(buf1, 0, strlen(buf1), oaffects, 0);
    mod = atoi(buf2);

    if (type == -1) {
	send_to_char("That affect does not exist.\n\r", ch);
	return;
    }
    if (!mod) {
	send_to_char("You must specify an amount to affect it by.\n\r", ch);
	return;
    }
    if ((mod > 100) || (mod < -100)) {
	send_to_char("That object affect is too high.\n\r", ch);
	return;
    }
    obj->affected[a].location = type;
    obj->affected[a].modifier = mod;
    send_to_char("Done.\n\r", ch);
}


void do_oset(struct char_data *ch, char *argument, int cmd)
{
    char arg1[256], arg2[256], arg3[256];
    char buf[256], buf2[256];
    int i, j;
    int val, dice, sides, dir;
    int value, spaces;
    struct obj_data *obj;
    struct extra_descr_data *tmpexd;

    char *generic_field[] = {
        "name",
        "sdesc",
        "ldesc",
        "desc",
        "edesc",
        "type",
        "aff1",
        "aff2",
        "weight",
        "cost",
        "storage",
        "worn_type",
        "item",
        "\n"
    };

    char *generic_desc[] = {
        "(\"sword long spiked\")",
        "(\"a spiked long sword\")",
        "(\"You see a spiked long sword here.\")",
        "(object description)",
        "(object extra description)",
        "(object type)",
        "(first affect)",
        "(second affect)",
        "(object weight)",
        "(object cost)",
        "(object storage cost)",
        "(where it can be worn(take is included in all of them)",
        "flags like ANTI_GOOD",
        "\n"
    };

    extern char *drinknames[];
    extern char *spells[];

    argument = one_argument(argument, arg1);
    half_chop(argument, arg2, arg3);

    obj = get_obj_vis(ch, arg1);
    if (!obj) {
	send_to_char("That object does not exist.\n\r", ch);
	return;
    }


    if (!*arg2) {
        for (i =  0; *generic_field[i] != '\n'; i++) {
            spaces = 15 - strlen(generic_field[i]);
            sprintf(buf2, "");
            for (j = 0; j < spaces; j++) 
                strcat(buf2, " ");
            sprintf(buf, "%s%s%s\n\r", generic_field[i], buf2, generic_desc[i]);
            send_to_char(buf, ch);
        }
        for (i = 0; i < 4; i++) {
            if (strcmp(oset_field[obj->obj_flags.type_flag].set[i], "")) {
                spaces = 15 - strlen(oset_field[obj->obj_flags.type_flag].set[i]);
                sprintf(buf2, "");
                for (j = 0; j < spaces; j++)
                    strcat(buf2, " ");
                sprintf(buf, "%s%s%s\n\r", oset_field[obj->obj_flags.type_flag].set[i], buf2, oset_field[obj->obj_flags.type_flag].description[i]);
                send_to_char(buf, ch);
            }
        }
        return;
    }

    for (i = 0; *generic_field[i] != '\n'; i++)
        if (!strncasecmp(generic_field[i], arg2, strlen(arg2))) break;

    switch (i) {
    case 0: /* name */
        for (tmpexd = obj->ex_description; tmpexd; tmpexd = tmpexd->next)
            if (!strcasecmp(tmpexd->keyword, obj->name))
	        break;

        if (tmpexd) {
            free((char *) tmpexd->keyword);
            tmpexd->keyword = strdup(arg3);
        }
        free((char *) obj->name);
        obj->name = strdup(arg3);
        send_to_char("Done.\n\r", ch);
        return;
    case 1: /* sdesc */
        free((char *)obj->short_description);
        obj->short_description = strdup(arg3);
        send_to_char("Done.\n\r", ch);
        return;
    case 2: /* ldesc */
        free((char *) obj->description);
        obj->description = strdup(arg3);
        send_to_char("Done.\n\r", ch);
        return;
    case 3: /* desc */
        sprintf(buf, " %s", obj->name);
        set_oedesc(ch, obj, buf);
        return;
    case 4: /* edesc */
        set_oedesc(ch, obj, arg3);
        return;
    case 5: /* type */
        set_otype(ch, obj, arg3);
        return;
    case 6: /* aff1 */
        set_oaffect(ch, obj, arg3, 0);
        return;
    case 7: /* aff2 */
        set_oaffect(ch, obj, arg3, 1);
        return;
    case 8: /* weight */
        if ((atoi(arg3) < 0) || (atoi(arg3) > 100000)) {
            send_to_char("That weight is invalid.\n\r", ch);
            return;
        }
        obj->obj_flags.weight = atoi(arg3);
        send_to_char("Done.\n\r", ch);
        return;
    case 9: /* cost */
        if ((atoi(arg3) < 0) || (atoi(arg3) > 100000)) {
            send_to_char("That cost is invalid.\n\r", ch);
            return;
        }
        obj->obj_flags.cost = atoi(arg3);
        send_to_char("Done.\n\r", ch);
        return;
    case 10: /* storage */
        if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
            send_to_char("That storage cost is invalid.\n\r", ch);
            return;
        }
        obj->obj_flags.cost_per_day = atoi(arg3);
        send_to_char("Done.\n\r", ch);
        return;
    case 11: /* worn type */
        set_mtype(ch, obj, arg3);
        return;
    case 12: /* extra flags */
        set_oflags(ch, obj, arg3);
        return;
    }

    for (i = 0; i < 4; i++) 
        if (!strncasecmp(oset_field[obj->obj_flags.type_flag].set[i], arg2, strlen(arg2))) break;
    if (i == 4) {
        send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
        return;
    }

    switch (obj->obj_flags.type_flag) {
    case ITEM_LIGHT:
        switch (i) {
        case 0: /* duration */
            if (!*arg3) {
                send_to_char("ITEM_LIGHT: DURATION\n\r\n\rThe light's duration is the amount of time (in game hours) that the\n\rlight will stay on.  Use a value of -1 for an infinite duration.\n\r", ch);
                return;
            } 
            if ((atoi(arg3) > 100000) || (atoi(arg3) < -1)) {
                send_to_char("That light duration is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_SCROLL:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_SCROLL: LEVEL\n\r\n\rThis value represents the level of the spells contained in the scroll.\n\rIt must be a number between 1 and 20.\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 1: /* spell */
        case 2:
        case 3:
            if (!*arg3) {
                send_to_char("ITEM_SCROLL: SPELL\n\r\n\rThis value determines which spell is performed by reciting the scroll.\n\rYou need not use the spell number, but simply the name:\n\rOSET <scroll> SPELL# FIREBALL\n\r", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n\r", ch);
	        return;
	    }
	    val--;
            obj->obj_flags.value[i] = val;
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_WAND:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_WAND: LEVEL\n\r\n\rThis value represents the level of the spell contained in the wand.\n\rIt must be a number between 1 and 20.\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 1: /* max-charges */
            if (!*arg3) {
                send_to_char("ITEM_WAND: MAX-CHARGES\n\r\n\rThe number of \"charges\" that a wand has is the amount of times that it\n\rcan be used.  In the future a \"recharge\" spell will probably be made, which\n\ris probably the reason that a \"max-charges\" value was created in the first\n\rplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\n\runless you have something weird in mind. ;-)\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That max-charges value is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[1] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 2: /* charges */
            if (!*arg3) {
                send_to_char("ITEM_WAND: CHARGES\n\r\n\rThe number of \"charges\" that a wand has is the amount of times that it\n\rcan be used.  In the future a \"recharge\" spell will probably be made, which\n\ris probably the reason that a \"max-charges\" value was created in the first\n\rplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\n\runless you have something weird in mind. ;-)\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That charges value is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[2] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 3: /* spell */
            if (!*arg3) {
                send_to_char("ITEM_WAND: SPELL\n\r\n\rThis value determines which spell is performed by using the wand.\n\rYou need not use the spell number, but simply the name:\n\rOSET <wand> SPELL# FIREBALL\n\r", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n\r", ch);
	        return;
	    }
	    val--;
            obj->obj_flags.value[3] = val;
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_STAFF:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: LEVEL\n\r\n\rThis value represents the level of the spell contained in the staff.\n\rIt must be a number between 1 and 20.\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 1: /* max-charges */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: MAX-CHARGES\n\r\n\rThe number of \"charges\" that a staff has is the amount of times that it\n\rcan be used.  In the future a \"recharge\" spell will probably be made, which\n\ris probably the reason that a \"max-charges\" value was created in the first\n\rplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\n\runless you have something weird in mind. ;-)\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That max-charges value is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[1] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 2: /* charges */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: CHARGES\n\r\n\rThe number of \"charges\" that a staff has is the amount of times that it\n\rcan be used.  In the future a \"recharge\" spell will probably be made, which\n\ris probably the reason that a \"max-charges\" value was created in the first\n\rplace.  Just set \"max-charges\" and \"charges\" to the same thing for now,\n\runless you have something weird in mind. ;-)\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That charges value is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[2] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 3: /* spell */
            if (!*arg3) {
                send_to_char("ITEM_STAFF: SPELL\n\r\n\rThis value determines which spell is performed by using the staff.\n\rYou need not use the spell number, but simply the name:\n\rOSET <staff> SPELL# FIREBALL\n\r", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n\r", ch);
	        return;
	    }
	    val--;
            obj->obj_flags.value[3] = val;
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_WEAPON:
        switch (i) {
        case 0:
            if (!*arg3) {
                send_to_char("ITEM_WEAPON: DAMAGE\n\r\n\rWeapon damage *must* be given in the form \"XdY\", where X and Y are the\n\rtwo damage dice.  A die with Y sides is rolled X times, and added up for the\n\rtotal damage.  Other bonuses such as character strength also apply to\n\rdamage.  Damage should range from around 1d4 at a minimum to 5d5 or so as a\n\rmaximum.  If you wish to create a weapon which is an exception to the rules,\n\rtalk to Brutius about it.\n\r", ch);
                return;
            }
            sscanf(arg3, "%dd%d", &dice, &sides);
            if ((dice < 1) || (dice > 100) || (sides < 1) || (sides > 100)) {
                send_to_char("That weapon damage is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[1] = dice;
            obj->obj_flags.value[2] = sides;
            send_to_char("Done.\n\r", ch);
            break;
        case 1: /* wtype */
            if (!*arg3) {
                send_to_char("ITEM_WEAPON: WTYPE\n\r\n\rThe weapon type determines the attack type used by the weapon.\n\rYou may set it to (p)iercing, (s)lashing, (b)ludgeoning,\n\r(w)hipping, (c)lawwing, bit(i)ng, sti(n)ging\n\rc(r)ushing, c(l)eaving, s(t)abbing, smas(h)ing, s(m)iting", ch);
                return;
            if ((*arg3 == 'p') || (*arg3 == 'P')) 
               obj->obj_flags.value[3] = TYPE_PIERCE;
            else if ((*arg3 == 's') || (*arg3 == 'S')) 
               obj->obj_flags.value[3] = TYPE_SLASH;
            else if ((*arg3 == 'b') || (*arg3 == 'B')) 
               obj->obj_flags.value[3] = TYPE_BLUDGEON;
            else if ((*arg3 == 'w') || (*arg3 == 'W')) 
               obj->obj_flags.value[3] = TYPE_WHIP;
            else if ((*arg3 == 'c') || (*arg3 == 'C')) 
               obj->obj_flags.value[3] = TYPE_CLAW;
            else if ((*arg3 == 'i') || (*arg3 == 'I')) 
               obj->obj_flags.value[3] = TYPE_BITE;
            else if ((*arg3 == 'n') || (*arg3 == 'N')) 
               obj->obj_flags.value[3] = TYPE_STING;
            else if ((*arg3 == 'r') || (*arg3 == 'R')) 
               obj->obj_flags.value[3] = TYPE_CRUSH;
            else if ((*arg3 == 'l') || (*arg3 == 'L')) 
               obj->obj_flags.value[3] = TYPE_CLEAVE;
            else if ((*arg3 == 't') || (*arg3 == 'T')) 
               obj->obj_flags.value[3] = TYPE_STAB;
            else if ((*arg3 == 'h') || (*arg3 == 'H')) 
               obj->obj_flags.value[3] = TYPE_SMASH;
            else if ((*arg3 == 'm') || (*arg3 == 'M')) 
               obj->obj_flags.value[3] = TYPE_SMITE;
            else {
                send_to_char("That is not a valid weapon type.\n\r", ch);
                return;
            }
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_ARMOR:
        switch (i) {
        case 0: /* points */
            if (!*arg3) {
                send_to_char("ITEM_ARMOR: POINTS\n\r\n\rArmor points are directly proportional to the amount of damage absorbed\n\rby a blow from a weapon on that position.  If the armor points value is\n\rdivisible by ten, the armor will absord points / 10 damage points.  If it is\n\rin between multiples of ten, the amount will be semi-random.  For instance,\n\rsay a helmet of 44 AP is hit for 10 damage.  There will be a 40% chance of\n\rthe armor absorbing 5 damage points, and a 60% chance of it absorbing 4.\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 100)) {
                send_to_char("That armor points value is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_POTION:
        switch (i) {
        case 0: /* level */
            if (!*arg3) {
                send_to_char("ITEM_POTION: LEVEL\n\r\n\rThis value represents the level of the spells contained in the potion.\n\rIt must be a number between 1 and 20.\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 1) || (atoi(arg3) > 20)) {
                send_to_char("That spell level is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 1: /* spell */
        case 2:
        case 3:
            if (!*arg3) {
                send_to_char("ITEM_POTION: SPELL\n\r\n\rThis value determines which spell is performed by quaffing the potion.\n\rYou need not use the spell number, but simply the name:\n\rOSET <potion> SPELL# FIREBALL\n\r", ch);
                return;
            }
	    val = (old_search_block(arg3, 0, strlen(arg3), spells, 0));
	    if (val == -1) {
	        send_to_char("There is no spell of that name.\n\r", ch);
	        return;
	    }
	    val--;
            obj->obj_flags.value[i] = val;
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_CONTAINER:
        switch (i) {
        case 0: /* capacity */
            if (!*arg3) {
                send_to_char("ITEM_CONTAINER: CAPACITY\n\r\n\rThis value determines the maximum weight that the container can hold.\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
                send_to_char("That capacity is invalid.\n\r", ch);
                return; 
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 1: /* flags */
            if (!*arg3) {
                send_to_char("ITEM_CONTAINER: FLAGS\n\r\n\rYou may set the following flags: (c)losable, (p)ickproof, and (t)rapped.\n\r", ch);
                return;
            }
            switch (*arg3) {
            case 'c':
            case 'C':
                if (IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE)) {
                    REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSEABLE);
                    send_to_char("You have removed the 'closable' flag\n\r", ch);    
                } else {
                    SET_BIT(obj->obj_flags.value[1], CONT_CLOSEABLE);
                    send_to_char("You have set the 'closable' flag\n\r", ch);  
                }
                break;
            case 'p':
            case 'P':
                if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
                    REMOVE_BIT(obj->obj_flags.value[1], CONT_PICKPROOF);
                    send_to_char("You have removed the 'pickproof' flag\n\r", ch);    
                } else {
                    SET_BIT(obj->obj_flags.value[1], CONT_PICKPROOF);
                    send_to_char("You have set the 'pickproof' flag\n\r", ch);  
                }
                break;
            }
            break;
        case 2: /* key */
            if (!*arg3) {
                send_to_char("ITEM_CONTAINER: KEY\n\r\n\rThis value is the virtual object number for a key which can open the\n\rcontainer.\n\r", ch);
                return;
            }
            obj->obj_flags.value[2] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
    case ITEM_DRINKCON:
        switch (i) {
        case 0: /* capacity */
            if (!*arg3) {
                send_to_char("ITEM_DRINKCON: CAPACITY\n\r\n\rThis value represents the amount of liquid that the drink container\n\rcan hold.\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
                send_to_char("That capacity is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 1:
            if (!*arg3) {
               send_to_char("ITEM_DRINKCON: AMOUNT\n\r\n\rThis is the amount of liquid left in the container.\n\r", ch);
               return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 10000)) {
                send_to_char("That amount is invalid.\n\r", ch);
                return;
            }
            if (atoi(arg3) > obj->obj_flags.value[0]) {
                send_to_char("The amount of liquid can't be more than the liquid capacity.\n\r", ch);
                return;
            }
            obj->obj_flags.value[1] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 2:
            if (!*arg3) {
                send_to_char("ITEM_DRINKCON: TYPE\n\r\n\rThere are a variety of different drink types you can use.  Read HELP\n\rITEM_DRINKCON for a list of them all.\n\r", ch);
                return;
            }
            for (i = 0; i < 16; i++)
                if (!strncasecmp(arg3, drinknames[i], strlen(arg3))) break;
            if (i == 16) {
                send_to_char("There is no such drink.  See HELP ITEM_DRINKCON for a list.\n\r", ch);
                return;
            }
            obj->obj_flags.value[2] = i;
            send_to_char("Done.\n\r", ch);
            break;
        case 3:
            if (!*arg3) {
                send_to_char("ITEM_DRINKCON: POISONED\n\r\n\rSet this value to TRUE if you want the drink to be poisoned.\n\r", ch);
                return;
            }
            if (!strncasecmp(arg3, "true", strlen(arg3))) {
                obj->obj_flags.value[3] = TRUE;
                send_to_char("Done.\n\r", ch);
            } else if (!strncasecmp(arg3, "false", strlen(arg3))) {
                obj->obj_flags.value[3] = FALSE;
                send_to_char("Done.\n\r", ch);
            } else 
                send_to_char("The poisoned setting can be set to either TRUE or FALSE.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_FOOD:
        switch (i) {
        case 0:
            if (!*arg3) {
                send_to_char("ITEM_FOOD: AMOUNT\n\r\n\rThis value represents the number of game hours which the food will fill\n\ryou for.\n\r", ch);
                return;
            }
            if ((atoi(arg3) > 24) || (atoi(arg3) < 1)) {
                send_to_char("That amount is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[i] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        case 1:
            if (!*arg3) {
                send_to_char("ITEM_FOOD: POISONED\n\r\n\rSet this value to TRUE if you want the drink to be poisoned.\n\r", ch);
                return;
            }
            if (!strncasecmp(arg3, "true", strlen(arg3))) {
                obj->obj_flags.value[3] = TRUE;
                send_to_char("Done.\n\r", ch);
            } else if (!strncasecmp(arg3, "false", strlen(arg3))) {
                obj->obj_flags.value[3] = FALSE;
                send_to_char("Done.\n\r", ch);
            } else 
                send_to_char("The poisoned setting can be set to either TRUE or FALSE.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    case ITEM_MONEY:
        switch (i) {
        case 0:
            if (!*arg3) {
                send_to_char("ITEM_MONEY: AMOUNT\n\r\n\rThe amount of gold contained in a \"gold object.\"\n\r", ch);
                return;
            }
            if ((atoi(arg3) < 0) || (atoi(arg3) > 1000000)) {
                send_to_char("That amount is invalid.\n\r", ch);
                return;
            }
            obj->obj_flags.value[0] = atoi(arg3);
            send_to_char("Done.\n\r", ch);
            break;
        default:
            send_to_char("That field is invalid.  Type OSET <object> with no arguments for more info.\n\r", ch);
            break;
        }
        return;
    }
 }
}
        
        
                

