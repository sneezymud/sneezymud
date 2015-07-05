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
-- Table structure for table `shopownedplayer`
--

DROP TABLE IF EXISTS `shopownedplayer`;
CREATE TABLE `shopownedplayer` (
  `shop_nr` int(11) default NULL,
  `player` varchar(128) default NULL,
  `profit_buy` double default NULL,
  `profit_sell` double default NULL,
  `max_num` int(11) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `shopownedplayer`
--


/*!40000 ALTER TABLE `shopownedplayer` DISABLE KEYS */;
LOCK TABLES `shopownedplayer` WRITE;
INSERT INTO `shopownedplayer` VALUES (70,'hinata',1.2,0.4,1),
(70,'adoresqua',1.2,0.4,1),
(70,'sularia',1.2,0.4,1),
(70,'juliette',1.2,0.4,1),
(139,'Sephie',1,0.5,10),
(111,'frobozz',5,0.01,0),
(70,'raven',2,1.3,10),
(137,'Gon',1,0.6,10),
(137,'Millie',1,0.6,10),
(139,'Ragamuffin',1,0.5,10),
(137,'Risk',1,0.6,10),
(60,'frobozz',5,0.01,1),
(60,'life',5,0.01,1),
(139,'Millie',1,0.5,10),
(34,'pirvan',5,0.01,0),
(138,'Brennar',0.01,0.01,1),
(66,'IRS',0.01,0.01,1),
(139,'Taenryl',1,0.5,10),
(139,'Thag',1,0.5,10),
(136,'Knuckles',0,0,6),
(139,'ShamZami',1,0.5,10),
(137,'Torm',1,0.6,10),
(137,'Dayn',1,0.6,10),
(70,'frobozz',5,0.01,1),
(70,'araxus',5,0.01,1),
(70,'solarius',5,0.01,1),
(70,'ravem',5,0.01,1),
(70,'dot',5,0.01,1),
(70,'sephie',5,0.01,1),
(70,'coppern',5,0.01,1),
(70,'drakealgae',5,0.01,1),
(70,'blindfate',5,0.01,1),
(70,'amberbock',5,0.01,1),
(34,'blindfate',5,0.01,1),
(34,'drakealgae',5,0.01,1),
(34,'frobozz',5,0.01,1),
(34,'killjoy',5,0.01,1),
(34,'rouge',5,0.01,1),
(34,'knuckles',5,0.01,1),
(34,'amberbock',5,0.01,1),
(34,'kilian',5,0.01,1),
(34,'solarius',5,0.01,1),
(34,'araxus',5,0.01,1),
(137,'ShamZami',1,0.6,10),
(138,'Lilslugger',0.01,0.01,10),
(111,'peewee',5,0.1,0),
(34,'selch',5,0.1,0),
(138,'Khulshalkrum',0.01,0.01,1),
(138,'KhulShalkrum',0.01,0.01,1),
(35,'selch',5,0.1,0),
(70,'selch',5,0.1,0),
(34,'lorrheek',5,0.01,1),
(138,'Magnito',0.01,0.01,1),
(139,'Gon',1,0.5,10),
(139,'Rouge',1,0.5,10),
(139,'Egg',1,0.5,10),
(139,'Jiraiya',1,0.5,10),
(139,'Peewee',1,0.5,10),
(139,'Cirk',1,0.5,10),
(139,'Haoma',1,0.5,10),
(34,'garush',5,0.01,1),
(139,'Selch',1,0.5,10),
(139,'Freedom',1,0.5,10),
(139,'Shakudo',1,0.5,10),
(139,'LilSlugger',1,0.5,10),
(138,'Devlin',0.001,0.001,1),
(138,'Rome',0.1,0.1,10),
(138,'Dhamapala',0.01,0.01,1),
(138,'Ethelric',0.1,0.1,10),
(138,'Thrash',0.1,0.1,10),
(138,'Micelli',0.1,0.1,10),
(138,'Eshejar',0.1,0.1,10),
(138,'Buyirugh',0.1,0.1,10),
(138,'Ying',0.1,0.1,10),
(138,'Khaleefa',0.1,0.1,10),
(138,'Frodo',0.1,0.1,10),
(138,'Millian',0.1,0.1,10),
(138,'Imran',0.1,0.1,10),
(139,'String',1,0.5,10),
(138,'Delvin',0.001,0.001,1),
(137,'Egg',1,0.6,10),
(137,'Beloc',1,0.6,10),
(137,'Thag',1,0.6,10),
(137,'Legato',1,0.6,10),
(137,'Heal',1,0.6,10),
(137,'Cirk',1,0.6,10),
(137,'Peewee',1,0.6,10),
(137,'Jozan',1,0.6,10),
(137,'Eternity',1,0.6,10),
(137,'Roku',1,0.6,10),
(137,'Grogg',1,0.6,10),
(137,'Rouge',1,0.6,10),
(137,'Thibor',1,0.6,10),
(137,'Ragamuffin',1,0.6,10),
(137,'Trask',1,0.6,10),
(137,'Aiolos',1,0.6,10),
(137,'Xalami',1,0.6,10),
(137,'Sephie',1,0.6,10),
(137,'Bebo',1,0.6,10),
(137,'Wyntre',1,0.6,10),
(137,'LilSlugger',1,0.6,10),
(137,'Jiraiya',1,0.6,10),
(137,'Haoma',1,0.6,10),
(137,'Selch',1,0.6,10),
(139,'Cirius',1,0.5,10),
(139,'Zankou',1,0.5,10),
(139,'Pho',1,0.5,10),
(137,'Tre',1,0.6,10),
(137,'Pho',1,0.6,10),
(137,'Blum',1,0.6,10),
(137,'Emma',1,0.6,10),
(139,'Grimm',1,0.5,10),
(139,'Emma',1,0.5,10),
(139,'Coppern',1,0.5,10),
(137,'Coppern',1,0.6,10),
(54,'antonius',5,0.01,1),
(139,'Nebuchadnezzar',1,0.5,10),
(137,'Nebuchadnezzar',1,0.6,10),
(137,'Calypso',1,0.6,10),
(138,'Medic',0.001,0.001,1),
(137,'Sonic',1,0.6,10),
(139,'Sonic',1,0.5,10),
(44,'IRS',0.001,0.001,1),
(137,'Cirahn',1,0.6,10),
(137,'String',1,0.6,10),
(137,'Stin',1,0.6,10),
(122,'Animus',1,0.5,9999),
(122,'Theef',1,0.5,9999),
(122,'Fallian',1,0.5,9999),
(138,'IRS',0.001,0.001,1),
(139,'Ratfink',1,0.5,10),
(137,'Ratfink',1,0.6,10),
(139,'Hardpad',1,0.5,10),
(137,'Hardpad',1,0.6,10),
(137,'Taenryl',0.1,0.1,100);
UNLOCK TABLES;
/*!40000 ALTER TABLE `shopownedplayer` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

