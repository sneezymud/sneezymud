ALTER TABLE `mob`
    ADD `owner` char(32) default NULL,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY  (`owner`,`vnum`);

ALTER TABLE `mob_extra`
    ADD `owner` varchar(32) default NULL,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY  (`owner`,`vnum`,`keyword`);

ALTER TABLE `mob_imm`
    ADD `owner` varchar(32) default NULL,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY  (`owner`,`vnum`,`type`);

ALTER TABLE `mobresponses`
    ADD `owner` varchar(32) default NULL;

ALTER TABLE `obj`
    ADD `owner` varchar(32) default NULL,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY  (`owner`,`vnum`);

ALTER TABLE `objaffect`
    ADD `owner` varchar(32) default NULL;

ALTER TABLE `objextra`
    ADD `owner` varchar(32) default NULL;

ALTER TABLE `room`
    ADD `owner` varchar(32) default NULL,
    ADD `block` int(11) default NULL,
    DROP PRIMARY KEY,
    ADD PRIMARY KEY  (`owner`,`vnum`);

ALTER TABLE `roomexit`
    ADD `owner` varchar(32) default NULL,
    ADD `block` int(11) default NULL;

ALTER TABLE `roomextra`
    ADD `owner` varchar(32) default NULL,
    ADD `block` int(11) default NULL;
