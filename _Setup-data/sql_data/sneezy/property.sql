-- MariaDB dump 10.18  Distrib 10.5.8-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: sneezy-db    Database: sneezy
-- ------------------------------------------------------
-- Server version	10.5.8-MariaDB-1:10.5.8+maria~focal

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `property`
--

DROP TABLE IF EXISTS `property`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `property` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(80) DEFAULT NULL,
  `owner` int(11) DEFAULT NULL,
  `key_vnum` int(11) DEFAULT NULL,
  `entrance` int(11) DEFAULT NULL,
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=52 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `property`
--

LOCK TABLES `property` WRITE;
/*!40000 ALTER TABLE `property` DISABLE KEYS */;
INSERT INTO `property` VALUES (1,'NW Market Square Apartment',NULL,562,580),
(2,'NE Twilight Square Apartment',NULL,636,618),
(3,'5 Redlight Lane',NULL,4474,4626),
(4,'3 Redlight Lane',NULL,4624,4624),
(5,'W Twilight Avenue Apartment',NULL,642,636),
(6,'Haon-Dor Treehouse',NULL,1177,1177),
(7,'Jungle Flat',NULL,35450,9723),
(8,'Pleasant Acres',NULL,18732,18730),
(9,'Hammer and Fist Construction Co.',NULL,8883,33270),
(10,'Adrielz Tower',NULL,3866,3867),
(11,'Skull Duggory\'s Tower',NULL,3869,3870),
(12,'Haon-Dor Cabin',NULL,33290,33290),
(13,'Ancient Coffin',NULL,9648,9649),
(15,'A Ramshackle Backroom',NULL,16755,16707),
(32,'Farm',NULL,33579,33577),
(33,'Farmhouse',NULL,33580,33560),
(34,'The Tequila Sunrise',NULL,19003,19000),
(35,'A Hollow Magma Chamber',NULL,33652,9725),
(36,'A Cramped Attic',NULL,16700,16700),
(37,'Usurped Galek Suite',NULL,26879,26879),
(38,'Usurped Unaffliated Suite',NULL,26880,26880),
(39,'Usurped Serpent Suite',NULL,26881,26881),
(40,'Usurped Logrus Suite',NULL,26882,26882),
(41,'Private Room in Frontier Barracks',NULL,16724,16702),
(42,'A Raven\'s Nest',NULL,16741,16705),
(43,'Serpent\'s Overlord Sanctum',NULL,33295,33295),
(46,'Unfinished Bungalow On Stilts',NULL,16763,16708),
(47,'Oarsmen Hull',NULL,15381,15381),
(49,'The Great House',NULL,16765,10135),
(50,'A Simple Apartment',NULL,16767,16711),
(51,'Lord Deslok\'s Chamber',NULL,16768,16712);
/*!40000 ALTER TABLE `property` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-05-03 20:19:00
