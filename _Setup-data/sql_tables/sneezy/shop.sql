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
-- Table structure for table `shop`
--

DROP TABLE IF EXISTS `shop`;
CREATE TABLE `shop` (
  `shop_nr` int(11) NOT NULL default '0',
  `profit_buy` double NOT NULL default '0',
  `profit_sell` double NOT NULL default '0',
  `no_such_item1` varchar(127) NOT NULL default '',
  `no_such_item2` varchar(127) NOT NULL default '',
  `do_not_buy` varchar(127) NOT NULL default '',
  `missing_cash1` varchar(127) NOT NULL default '',
  `missing_cash2` varchar(127) NOT NULL default '',
  `message_buy` varchar(127) NOT NULL default '',
  `message_sell` varchar(127) NOT NULL default '',
  `temper1` int(11) NOT NULL default '0',
  `temper2` int(11) NOT NULL default '0',
  `keeper` int(11) NOT NULL default '0',
  `flags` int(11) NOT NULL default '0',
  `in_room` int(11) NOT NULL default '0',
  `open1` int(11) NOT NULL default '0',
  `close1` int(11) NOT NULL default '0',
  `open2` int(11) NOT NULL default '0',
  `close2` int(11) NOT NULL default '0',
  `expense_ratio` double default NULL,
  PRIMARY KEY  (`shop_nr`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
