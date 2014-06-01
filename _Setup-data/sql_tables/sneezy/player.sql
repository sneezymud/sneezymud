CREATE TABLE player (
    id serial NOT NULL,
    name varchar(80),
    talens integer,
    title varchar(80),
    account_id integer,
    guild_id integer,
    guildrank integer,
    load_room integer,
    last_logon integer
);

create unique index player_unq_name on player (name);
