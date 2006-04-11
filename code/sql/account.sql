CREATE TABLE account (
    account_id serial NOT NULL,
    email varchar(80),
    passwd varchar(13),
    name varchar(80),
    birth integer,
    term integer,
    time_adjust integer,
    flags integer,
    last_logon integer
);
