CREATE TABLE `mob_extra` (
  `owner` varchar(80) NOT NULL,
  `vnum` int(11) default NULL,
  `keyword` char(32) default NULL,
  `description` char(255) default NULL,
  PRIMARY KEY  (`owner`, `vnum`, `keyword`)
);
