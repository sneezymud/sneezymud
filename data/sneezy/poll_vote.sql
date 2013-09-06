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
-- Table structure for table `poll_vote`
--

DROP TABLE IF EXISTS `poll_vote`;
CREATE TABLE `poll_vote` (
  `account` varchar(80) NOT NULL,
  `poll_id` int(11) NOT NULL,
  `option_id` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `poll_vote`
--


/*!40000 ALTER TABLE `poll_vote` DISABLE KEYS */;
LOCK TABLES `poll_vote` WRITE;
INSERT INTO `poll_vote` VALUES ('Thark',1,6),
('Thark',2,10),
('bneely',1,8),
('Flagg',1,8),
('gabe',2,1),
('Ptok',2,7),
('sukhmam',2,1),
('sukhmam',1,8),
('Legato',1,7),
('trav',3,1),
('Langrad',3,2),
('Langrad',4,2),
('bneely',4,2),
('bneely',3,2),
('Flagg',2,8),
('Legato',4,1),
('trav',1,8),
('trav',2,1),
('Dreq',1,8),
('Dreq',2,1),
('Langrad',1,1),
('Langrad',2,10),
('crys',3,2),
('scout',3,1),
('crys',4,2),
('scout',4,2),
('schmutz',4,2),
('dara',3,2),
('dara',4,2),
('gabehall',3,1),
('gabehall',4,2),
('Thark',4,1),
('Thark',3,2),
('Fucko',4,2),
('Fucko',3,2),
('Toby',4,2),
('Toby',3,1),
('trav',5,2),
('weef',5,2),
('dara',5,1),
('jeremy',4,1),
('jeremy',5,1),
('jeremy',3,1),
('hill',5,2),
('ruke',5,1),
('garfield',3,1),
('garfield',4,2),
('garfield',5,3),
('gumby',3,1),
('gumby',5,2),
('gumby',4,2),
('JungleJim',5,3),
('Kelranth',5,2),
('bneely',5,2),
('nicoli',5,3),
('nicoli',3,1),
('nicoli',4,2),
('Thark',5,1),
('Legato',5,2),
('josh',5,2),
('josh',4,2),
('puter',5,2),
('crys',5,2),
('dyehead',3,2),
('dyehead',4,1),
('dyehead',5,1),
('JungleJim',4,2),
('Toby',5,3),
('Laren',3,1),
('Laren',4,1),
('Laren',5,3),
('jacob',5,1),
('jacob',4,2),
('jacob',3,2),
('sidartha',6,0),
('sidartha',8,1),
('palmolive',8,2),
('Ptok',10,10),
('Ptok',9,12),
('Eternal',9,3),
('katish',11,1),
('katish',10,16),
('katish',9,12),
('kellyjs',9,1),
('kellyjs',10,14),
('kellyjs',11,1),
('maminglun',9,11),
('maminglun',10,16),
('garfield',11,1),
('Aiolos',11,1),
('Aiolos',10,14),
('Aiolos',9,11),
('palmolive',9,2),
('Laren',9,9),
('palmolive',10,14),
('palmolive',11,1),
('palmolive',12,2),
('jeremy',9,1),
('Laren',10,1),
('Ptok',12,4),
('jeremy',10,12),
('jeremy',12,3),
('jeremy',11,2),
('weef',9,10),
('weef',10,7),
('weef',11,1),
('weef',12,5),
('Kelranth',9,10),
('Kelranth',10,14),
('Kelranth',11,1),
('Kelranth',12,3),
('Legato',9,10),
('Legato',10,18),
('Legato',11,1),
('Legato',12,2),
('garfield',9,11),
('garfield',10,18),
('gumby',12,5),
('gumby',9,12),
('gumby',11,2),
('gumby',10,16),
('Wyzak',9,9),
('Wyzak',10,17),
('Wyzak',11,2),
('Wyzak',12,3),
('layne',9,1),
('layne',10,14),
('layne',12,4),
('sculpy',11,2),
('sculpy',12,4),
('sculpy',10,7),
('sculpy',9,5),
('sukhmam',12,3),
('sukhmam',9,2),
('sukhmam',11,1),
('sukhmam',10,14),
('crys',12,4),
('Toby',9,12),
('Toby',10,13),
('Toby',12,3),
('Legato',13,1),
('darkclaw',9,2),
('darkclaw',10,7),
('darkclaw',11,1),
('darkclaw',12,4),
('Stin',9,11),
('Stin',10,7),
('Stin',11,2),
('Stin',12,3),
('Stin',13,1),
('christine',9,11),
('christine',10,16),
('christine',12,3),
('Blum',9,10),
('Blum',10,7),
('Blum',12,3),
('crys',15,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `poll_vote` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

