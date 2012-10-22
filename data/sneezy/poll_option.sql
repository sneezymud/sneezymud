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
-- Table structure for table `poll_option`
--

DROP TABLE IF EXISTS `poll_option`;
CREATE TABLE `poll_option` (
  `option_id` int(11) NOT NULL,
  `poll_id` int(11) NOT NULL,
  `descr` varchar(127) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `poll_option`
--


/*!40000 ALTER TABLE `poll_option` DISABLE KEYS */;
LOCK TABLES `poll_option` WRITE;
INSERT INTO `poll_option` VALUES (1,1,'Peel'),(1,10,'Getting started with a new character.'),(3,1,'Maror'),(4,1,'Angus'),(5,1,'Damescena'),(6,1,'Lapsos'),(7,1,'Ekeron'),(1,12,'0-10'),(9,1,'Cosmo'),(10,1,'Batopr'),(1,2,'Dyehead'),(2,10,'Hitting level 50.'),(3,2,'Dyehead'),(4,2,'Dyehead'),(5,2,'Dyehead'),(6,2,'Dyehead'),(7,2,'Dyehead'),(2,12,'11-20'),(9,2,'Dyehead'),(10,2,'Dyehead'),(1,3,'Knuckles'),(3,10,'Hard to make money.'),(1,4,'Antonious'),(4,10,'Too easy to make money.'),(1,5,'Bush'),(5,10,'Hard to find equipment.'),(3,5,'Peel'),(6,10,'Too easy to find equipment.'),(7,10,'All the damn lowbies walking around in diamond.'),(1,8,'Jesus'),(3,12,'21-30'),(9,10,'Can\'t use most of the shops.'),(2,8,'sidartha'),(10,10,'The immortals don\'t fix problems.'),(11,10,'Not enough zones.'),(12,10,'Not enough classes.'),(1,9,'Getting started with a new character.'),(2,9,'Hitting level 50.'),(3,9,'Hard to make money.'),(4,9,'Easy to make money!'),(5,9,'Equipment is a challenge to obtain.'),(6,9,'Equipment is easy to obtain!'),(7,9,'Private ownership - shops.'),(4,12,'31-40'),(5,12,'41-50'),(8,9,'Private ownership - homes.'),(9,9,'The persistent world.'),(10,9,'The immortal staff - what a great bunch of VOLUNTEERS.'),(11,9,'The world - big enough, well made. Good atmosphere.'),(13,10,'Immortals are unresponsive.'),(12,9,'The other players.'),(14,10,'Where are the other players?'),(1,11,'yes'),(6,12,'51-60'),(15,10,'No one votes.'),(16,10,'Where are my multiclasses. *sniff*'),(17,10,'SneezyMUD is so HARD!'),(13,9,'Power leveling.'),(18,10,'Power leveling.'),(7,12,'61-70'),(8,12,'71-80'),(9,12,'81+'),(2,11,'who?'),(1,13,'Boo!'),(1,14,'Kelranth'),(1,15,'IRS'),(1,16,'Burglar');
UNLOCK TABLES;
/*!40000 ALTER TABLE `poll_option` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

