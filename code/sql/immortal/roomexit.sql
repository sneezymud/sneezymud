CREATE TABLE obj (
    vnum integer DEFAULT '0' NOT NULL,
    owner varchar(80) DEFAULT '' NOT NULL,
    name varchar(127) DEFAULT '' NOT NULL,
    short_desc varchar(127) DEFAULT '' NOT NULL,
    long_desc varchar(255) DEFAULT '' NOT NULL,
    action_desc varchar(255) DEFAULT '' NOT NULL,
    type integer DEFAULT '0' NOT NULL,
    action_flag integer DEFAULT '0' NOT NULL,
    wear_flag integer DEFAULT '0' NOT NULL,
    val0 integer DEFAULT '0' NOT NULL,
    val1 integer DEFAULT '0' NOT NULL,
    val2 integer DEFAULT '0' NOT NULL,
    val3 integer DEFAULT '0' NOT NULL,
    weight double precision DEFAULT '0.00' NOT NULL,
    price integer DEFAULT '0' NOT NULL,
    can_be_seen integer DEFAULT '0' NOT NULL,
    spec_proc integer DEFAULT '0' NOT NULL,
    max_exist integer DEFAULT '0' NOT NULL,
    max_struct integer DEFAULT '0' NOT NULL,
    cur_struct integer DEFAULT '0' NOT NULL,
    decay integer DEFAULT '0' NOT NULL,
    volume integer DEFAULT '0' NOT NULL,
    material integer DEFAULT '0' NOT NULL
);
CREATE TABLE objaffect (
    vnum integer DEFAULT '0' NOT NULL,
    owner varchar(80) DEFAULT '' NOT NULL,
    type integer DEFAULT '0' NOT NULL,
    mod1 integer DEFAULT '0' NOT NULL,
    mod2 integer DEFAULT '0' NOT NULL
);
CREATE TABLE objextra (
    vnum integer DEFAULT '0' NOT NULL,
    owner varchar(80) DEFAULT '' NOT NULL,
    name varchar(127) DEFAULT '' NOT NULL,
    description text NOT NULL
);
CREATE TABLE room (
    vnum integer NOT NULL,
    owner integer NOT NULL,
    x integer NOT NULL,
    y integer NOT NULL,
    z integer NOT NULL,
    name varchar(127) NOT NULL,
    description text NOT NULL,
    room_flag integer NOT NULL,
    sector integer NOT NULL,
    teletime integer NOT NULL,
    teletarg integer NOT NULL,
    telelook integer NOT NULL,
    river_speed integer NOT NULL,
    river_dir integer NOT NULL,
    capacity integer NOT NULL,
    height integer NOT NULL
);
CREATE TABLE roomextra (
    vnum integer NOT NULL,
    owner integer NOT NULL,
    name text NOT NULL,
    description text NOT NULL
);
CREATE TABLE roomexit (
    vnum integer NOT NULL,
    owner integer NOT NULL,
    direction integer NOT NULL,
    name varchar(127) NOT NULL,
    description text NOT NULL,
    type integer NOT NULL,
    condition_flag integer NOT NULL,
    lock_difficulty integer NOT NULL,
    weight integer NOT NULL,
    key_num integer NOT NULL,
    destination integer NOT NULL
);
