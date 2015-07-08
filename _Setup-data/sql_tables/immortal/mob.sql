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
-- Table structure for table `mob`
--

DROP TABLE IF EXISTS `mob`;
CREATE TABLE `mob` (
  `owner` varchar(32) default NULL,
  `vnum` int(11) NOT NULL,
  `name` char(127) NOT NULL,
  `short_desc` char(127) NOT NULL,
  `long_desc` char(255) NOT NULL,
  `description` text NOT NULL,
  `actions` int(11) unsigned NOT NULL,
  `affects` int(11) unsigned NOT NULL,
  `faction` int(11) NOT NULL,
  `fact_perc` int(11) NOT NULL,
  `letter` char(1) NOT NULL,
  `attacks` float(5,1) NOT NULL,
  `class` int(11) NOT NULL,
  `level` int(11) NOT NULL,
  `tohit` int(11) NOT NULL,
  `ac` float(5,1) NOT NULL,
  `hpbonus` float(5,1) NOT NULL,
  `damage_level` float(5,1) NOT NULL,
  `damage_precision` int(11) NOT NULL,
  `gold` int(11) NOT NULL,
  `race` int(11) NOT NULL,
  `weight` int(11) NOT NULL,
  `height` int(11) NOT NULL,
  `str` int(11) NOT NULL,
  `bra` int(11) NOT NULL,
  `con` int(11) NOT NULL,
  `dex` int(11) NOT NULL,
  `agi` int(11) NOT NULL,
  `intel` int(11) NOT NULL,
  `wis` int(11) NOT NULL,
  `foc` int(11) NOT NULL,
  `per` int(11) NOT NULL,
  `cha` int(11) NOT NULL,
  `kar` int(11) NOT NULL,
  `spe` int(11) NOT NULL,
  `pos` int(11) NOT NULL,
  `def_position` int(11) NOT NULL,
  `sex` int(11) NOT NULL,
  `spec_proc` int(11) NOT NULL,
  `skin` int(11) NOT NULL,
  `vision` int(11) NOT NULL,
  `can_be_seen` int(11) NOT NULL,
  `max_exist` int(11) NOT NULL,
  `local_sound` char(255) default NULL,
  `adjacent_sound` char(255) default NULL,
  PRIMARY KEY  (`owner`,`vnum`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
