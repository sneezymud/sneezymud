CREATE TABLE obj (
    vnum integer DEFAULT '0' NOT NULL,
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

INSERT INTO `obj` VALUES
(42,'placeholder object list silver orb small','<w>a small silver orb<1>','<w>A small silver orb floats here ominously.<1>','',12,2098240,16385,0,0,0,0,10,1,3,0,1,1,1,-1,1,63);
