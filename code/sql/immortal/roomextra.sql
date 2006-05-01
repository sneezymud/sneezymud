CREATE TABLE roomextra (
    vnum integer NOT NULL,
    owner varchar(80) DEFAULT '' NOT NULL,
    block integer NOT NULL,
    name text NOT NULL,
    description text NOT NULL
);
