alter table mob add owner varchar(80);
alter table mob_imm add owner varchar(80);
alter table mob_extra add owner varchar(80);
alter table mobresponses add owner varchar(80);

alter table obj add owner varchar(80);
alter table objaffect add owner varchar(80);
alter table objextra add owner varchar(80);

alter table room add owner varchar(80);
alter table room add block int;
alter table roomexit add owner varchar(80);
alter table roomexit add block int;
alter table roomextra add owner varchar(80);
alter table roomextra add block int;
