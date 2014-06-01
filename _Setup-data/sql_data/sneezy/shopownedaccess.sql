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
-- Table structure for table `shopownedaccess`
--

DROP TABLE IF EXISTS `shopownedaccess`;
CREATE TABLE `shopownedaccess` (
  `shop_nr` int(11) NOT NULL default '0',
  `name` varchar(80) NOT NULL default '',
  `access` int(11) NOT NULL default '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `shopownedaccess`
--


/*!40000 ALTER TABLE `shopownedaccess` DISABLE KEYS */;
LOCK TABLES `shopownedaccess` WRITE;
INSERT INTO `shopownedaccess` VALUES (115,'Melisandre',1),
(121,'aur',8),
(120,'sularia',78),
(120,'juliette',78),
(139,'gon',1),
(135,'Pitch',1),
(6,'spiff',16),
(115,'fedaykin',1),
(115,'frobozz',78),
(143,'fedaykin',1),
(121,'fingers',1),
(121,'pirvan',1),
(143,'AngelEyes',1),
(143,'taylor',1),
(0,'bass',1),
(143,'frobozz',64),
(125,'Fedaykin',64),
(120,'fedaykin',78),
(121,'immorality',1),
(183,'Nicoli',1),
(183,'Knuckles',1),
(0,'mee',14),
(181,'Ragamuffin',255),
(121,'Jozan',8),
(116,'Neon',127),
(120,'rind',78),
(121,'releep',8),
(137,'Gon',1),
(3,'GoldLama',66),
(0,'Froth',14),
(57,'Peel',1),
(56,'Peel',1),
(58,'Peel',1),
(81,'Peel',1),
(116,'Kavod',1),
(115,'Taylor',1),
(19,'knuckles',1),
(181,'Sephie',255),
(115,'AngelEyes',1),
(172,'pirvan',1),
(137,'Ragamuffin',255),
(184,'gon',66),
(184,'IRS',66),
(184,'kelranth',66),
(184,'burglar',66),
(139,'Ragamuffin',255),
(173,'troy',1),
(121,'troy',1),
(57,'knuckles',1),
(116,'peel',1),
(116,'dash',1),
(121,'jesus',1),
(4,'Peel',1),
(226,'hood',1),
(226,'mothergoose',1),
(226,'gloin',1),
(226,'ghoti',1),
(178,'troy',1),
(226,'Gidget',1),
(226,'Thorin',1),
(226,'Oin',1),
(115,'intuition',1),
(143,'intuition',1),
(143,'mysterious',1),
(115,'mysterious',1),
(226,'Beast',1),
(29,'karla',46),
(137,'Taenryl',1),
(137,'Cirius',1),
(22,'Hood',1),
(183,'Thorin',78),
(258,'Hood',1),
(258,'Thorin',1),
(258,'Ghoti',1),
(258,'Gloin',1),
(258,'MotherGoose',1),
(258,'Guppy',1),
(258,'Beast',1),
(257,'hood',1),
(257,'thorin',1),
(257,'ghoti',1),
(257,'gloin',1),
(257,'MotherGoose',1),
(257,'Guppy',1),
(257,'Beast',1),
(256,'hood',1),
(256,'thorin',1),
(256,'ghoti',1),
(256,'gloin',1),
(256,'MotherGoose',1),
(256,'Guppy',1),
(256,'Beast',1),
(172,'troy',1),
(226,'Pharaoh',1),
(256,'Pharaoh',1),
(257,'Pharaoh',1),
(258,'Pharaoh',1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `shopownedaccess` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

