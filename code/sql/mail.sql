CREATE TABLE mail (
    mailid integer DEFAULT nextval('mail_mailid_seq'::text) NOT NULL,
    port integer,
    mailfrom varchar(80),
    mailto varchar(80),
    timesent varchar(32),
    content varchar(4000)
);
