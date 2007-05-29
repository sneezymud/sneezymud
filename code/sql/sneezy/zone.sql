create table zone
(
zone_nr int(11) not null primary key,
zone_name varchar(255) not null default '',
zone_enabled int(11) null,
bottom int(11) null,
top int(11) null,
reset_mode int(11) null,
lifespan int(11) null,
age int(11) null,
util_flag int(11) null
)

