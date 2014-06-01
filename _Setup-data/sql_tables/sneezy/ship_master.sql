create table ship_master
(
	captain_vnum int not null,
	account_id int null,
	player_id int null
);
create index ix1_ship_master on ship_master (captain_vnum);
