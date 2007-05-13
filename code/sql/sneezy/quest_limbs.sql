CREATE TABLE `quest_limbs` (
  `player` varchar(80) NOT NULL,
  `team` varchar(30) default NULL,
  `mob_vnum` int(11) NOT NULL,
  `slot_num` int(11) NOT NULL,
  `slot_name` varchar(127) default NULL,
  `date_submitted` timestamp NOT NULL default CURRENT_TIMESTAMP
);
