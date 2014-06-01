use immortal;
alter table room add owner varchar(80);
alter table room add block int;
alter table roomexit add owner varchar(80);
alter table roomexit add block int;
alter table roomextra add owner varchar(80);
alter table roomextra add block int;
