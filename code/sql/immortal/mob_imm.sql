CREATE TABLE `mob_imm` (
  `owner` varchar(80) NOT NULL,
  `vnum` int(11) default NULL,
  `type` int(11) default NULL,
  `amt` int(11) default NULL,
  PRIMARY KEY  (`owner`, `vnum`, `type`)
);
