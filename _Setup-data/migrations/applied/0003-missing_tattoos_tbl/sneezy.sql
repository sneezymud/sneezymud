DROP TABLE IF EXISTS `tattoos`;
CREATE TABLE `tattoos` (
  `name` varchar(80) NOT NULL,
  `tattoo` varchar(128) NOT NULL,
  `location` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
