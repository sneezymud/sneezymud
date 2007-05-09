CREATE TABLE `mob_extra` (
  `vnum` int(11) default NULL,
  `keyword` char(32) default NULL,
  `description` char(255) default NULL,
  PRIMARY KEY  (`vnum`, `keyword`)
);
