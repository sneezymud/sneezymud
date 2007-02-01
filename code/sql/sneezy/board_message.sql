CREATE TABLE board_message (
  board_vnum int(11) NOT NULL,
  post_num int(11) default NULL,
  date_posted timestamp NOT NULL default CURRENT_TIMESTAMP,
  date_removed timestamp NULL default NULL,
  subject varchar(80) default NULL,
  author varchar(80) default NULL,
  post text,
  KEY ix__board_message__1 (board_vnum, date_removed)
);

