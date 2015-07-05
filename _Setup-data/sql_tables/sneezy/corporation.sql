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
-- Table structure for table `corporation`
--

DROP TABLE IF EXISTS `corporation`;
CREATE TABLE `corporation` (
  `corp_id` bigint(20) unsigned NOT NULL auto_increment,
  `name` varchar(80) NOT NULL,
  `bank` int(11) default NULL,
  UNIQUE KEY `corp_id` (`corp_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `corporation`
--


/*!40000 ALTER TABLE `corporation` DISABLE KEYS */;
LOCK TABLES `corporation` WRITE;
INSERT INTO `corporation` VALUES (1,'Small Business Alliance',4),
(2,'Regents of the University of KYA',4),
(3,'Fitz Inc.',4),
(4,'The Frobozz Magic Company',4),
(6,'Spiff Inc.',4),
(7,'The Four Spots Pirate Gang',142),
(8,'Adoresqua Inc.',4),
(10,'Metal Workers Union',4),
(11,'Buff and Chub Inc.',4),
(12,'Wicked Wonka Inc.',4),
(13,'GoldLama Co.',4),
(16,'Food Workers Union',4),
(17,'Players Alliance Against Crappy Shops',4),
(21,'Royal Kingdom of Grimhaven',4),
(23,'The Brotherhood of Galek',140),
(24,'The Cult of Logrus',141),
(25,'The Order of the Serpents',142),
(27,'Royal Kingdom of Amber',142),
(28,'Realm of the Cult of Logrus',141),
(29,'Honorable Kingdom of Brightmoon',140),
(30,'Sage Enterprises',4);
UNLOCK TABLES;
/*!40000 ALTER TABLE `corporation` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

