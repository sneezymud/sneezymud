CREATE TABLE querytimes (
    query varchar(512),
    secs double precision,
	date_logged timestamp default current_timestamp
);
