#ifndef __S_LIST_H
#define __S_LIST_H

void spell_burning_hands(byte level, struct char_data *ch, 
  struct char_data *victim, struct obj_data *obj);
void spell_call_lightning(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_chill_touch(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_shocking_grasp(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_colour_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_earthquake(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_energy_drain(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_fireball(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_harm(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_lightning_bolt(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_magic_missile(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
/*
  new spells
*/

void spell_cause_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_cause_critic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_flamestrike(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);


/*
  casts down below
*/

void cast_burning_hands( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_chill_touch( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_fireball( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_blindness( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sleep( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );

#endif
