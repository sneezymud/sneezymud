-- MySQL dump 10.10
--
-- Host: db.sneezymud.com    Database: sneezy
-- ------------------------------------------------------
-- Server version	5.0.24a-standard

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `obj`
--

DROP TABLE IF EXISTS `obj`;
CREATE TABLE `obj` (
  `vnum` int(11) NOT NULL default '0',
  `name` varchar(127) NOT NULL default '',
  `short_desc` varchar(127) NOT NULL default '',
  `long_desc` varchar(255) NOT NULL default '',
  `action_desc` varchar(255) NOT NULL default '',
  `type` int(11) NOT NULL default '0',
  `action_flag` int(11) NOT NULL default '0',
  `wear_flag` int(11) NOT NULL default '0',
  `val0` int(11) NOT NULL default '0',
  `val1` int(11) NOT NULL default '0',
  `val2` int(11) NOT NULL default '0',
  `val3` int(11) NOT NULL default '0',
  `weight` double NOT NULL default '0',
  `price` int(11) NOT NULL default '0',
  `can_be_seen` int(11) NOT NULL default '0',
  `spec_proc` int(11) NOT NULL default '0',
  `max_exist` int(11) NOT NULL default '0',
  `max_struct` int(11) NOT NULL default '0',
  `cur_struct` int(11) NOT NULL default '0',
  `decay` int(11) NOT NULL default '0',
  `volume` int(11) NOT NULL default '0',
  `material` int(11) NOT NULL default '0',
  PRIMARY KEY  (`vnum`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
