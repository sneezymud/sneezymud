CREATE TABLE poll_vote (
    account varchar(80) NOT NULL,
    poll_id integer NOT NULL,
    option_id integer NOT NULL
);
