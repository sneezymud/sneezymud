CREATE TABLE roomexit (
    vnum integer NOT NULL,
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
