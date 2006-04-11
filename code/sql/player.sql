CREATE TABLE player (
    id integer DEFAULT nextval('player_id_seq'::text) NOT NULL,
    name varchar(80),
    talens integer,
    title varchar(80),
    account_id integer
);
