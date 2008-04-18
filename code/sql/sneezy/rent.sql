drop table rent;
drop table rent_obj_aff;
drop table rent_strung;

create table rent (
  rent_id integer primary key auto_increment,
  owner_type ENUM('player', 'shop', 'room'),
  owner integer DEFAULT '0' NOT NULL,
  list_price integer DEFAULT '0' NOT NULL,
  slot integer DEFAULT '0' NOT NULL,
  vnum integer DEFAULT '0' NOT NULL,
  container integer DEFAULT '-1' NOT NULL,
  val0 integer DEFAULT '0' NOT NULL,
  val1 integer DEFAULT '0' NOT NULL,
  val2 integer DEFAULT '0' NOT NULL,
  val3 integer DEFAULT '0' NOT NULL,
  extra_flags integer DEFAULT '0' NOT NULL,
  weight double precision,
  bitvector integer DEFAULT '0' NOT NULL,
  decay integer DEFAULT '0' NOT NULL,
  cur_struct integer DEFAULT '0' NOT NULL,
  max_struct integer DEFAULT '0' NOT NULL,	
  material integer DEFAULT '0' NOT NULL,
  volume integer DEFAULT '0' NOT NULL,
  price integer DEFAULT '0' NOT NULL,
  depreciation integer DEFAULT '0' NOT NULL
);

create table rent_obj_aff (
  rent_id integer DEFAULT '0' NOT NULL,
  type integer DEFAULT '0' NOT NULL,
  level integer DEFAULT '0' NOT NULL,
  duration integer DEFAULT '0' NOT NULL,
  renew integer DEFAULT '0' NOT NULL,
  modifier integer DEFAULT '0' NOT NULL,
  location integer DEFAULT '0' NOT NULL,
  modifier2 integer DEFAULT '0' NOT NULL,
  bitvector integer DEFAULT '0' NOT NULL
);

create table rent_strung (
  rent_id integer DEFAULT '0' NOT NULL,
  name varchar(127) DEFAULT '' NOT NULL,
  short_desc varchar(127) DEFAULT '' NOT NULL,
  long_desc varchar(255) DEFAULT '' NOT NULL,
  action_desc varchar(255) DEFAULT '' NOT NULL
);
