CREATE TABLE mail (
    mailid serial NOT NULL,
    port integer,
    mailfrom varchar(80),
    mailto varchar(80),
    timesent varchar(32),
    content varchar(4000),
    talens integer,
    rent_id integer
);
