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
-- Table structure for table `factionmembers`
--

DROP TABLE IF EXISTS `factionmembers`;
CREATE TABLE `factionmembers` (
  `name` varchar(80) NOT NULL default '',
  `faction` varchar(8) default NULL,
  `level` int(11) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `factionmembers`
--


/*!40000 ALTER TABLE `factionmembers` DISABLE KEYS */;
LOCK TABLES `factionmembers` WRITE;
INSERT INTO `factionmembers` VALUES ('Auberon','snake',50),('Aphrodite','cult',50),('Arturs','snake',18),('Aiolos','cult',50),('Adoresqua','cult',50),('Astaldo','snake',23),('Anathema','snake',35),('Artur','cult',50),('Archie','snake',50),('Arlen','cult',50),('Aetrius','cult',50),('Burglar','cult',50),('Bunbun','cult',26),('Bumali','brother',42),('Bunny','cult',37),('Blackheart','cult',47),('Brennar','brother',50),('Behren','cult',27),('Coppern','brother',50),('Cairistiona','cult',10),('Cirk','cult',50),('Claw','snake',19),('Croup','snake',46),('DarkMagic','cult',21),('Deven','cult',19),('Dunlen','brother',50),('DeLorn','cult',48),('Diego','cult',17),('Ephitet','snake',38),('Euler','cult',50),('Flicka','snake',50),('Fortune','cult',50),('Fate','snake',24),('Fingers','cult',50),('Feno','cult',41),('Goronski','snake',31),('Gizmo','snake',50),('Gauge','cult',50),('Gidget','brother',33),('Grumble','brother',50),('Ganondorf','snake',50),('Galadriel','cult',17),('Heaven','snake',40),('Hellion','snake',15),('Hawkeye','snake',35),('Horror','cult',50),('Hell','snake',50),('Immorality','cult',47),('Icky','cult',50),('Isara','cult',25),('Intuition','cult',50),('Jelly','snake',50),('Jozan','cult',50),('Jokyl','snake',50),('Killjoy','snake',50),('Kriek','brother',22),('Kyt','cult',42),('LilNicky','snake',50),('Lichen','snake',16),('Loony','cult',50),('Lusion','cult',15),('Lorielle','cult',50),('Lovely','snake',21),('Mage','snake',50),('Mysterious','cult',50),('Mufasa','cult',9),('Mototaz','brother',50),('Massif','brother',12),('Miroku','cult',48),('Morte','cult',50),('MotherTeresa','cult',15),('Meatie','cult',50),('Nebuchadnezzar','brother',50),('Onni','cult',33),('Oden','snake',30),('Orochimaru','cult',50),('Okito','snake',14),('Precious','cult',8),('Ryedell','brother',24),('Rhians','cult',50),('Rock','snake',50),('Rick','snake',22),('Raknor','snake',50),('Roku','cult',47),('Smoldaeron','cult',26),('Stin','cult',47),('SupaFly','cult',42),('Skirmish','brother',50),('Strife','cult',50),('Slim','snake',19),('Stuhl','cult',11),('String','cult',41),('Splash','cult',50),('Shun','snake',36),('Strap','cult',29),('Slicer','cult',42),('Solarius','brother',50),('Seklor','cult',25),('Stinkypoo','snake',43),('Severus','cult',45),('Troy','cult',50),('Terrin','cult',45),('Tyson','snake',32),('Trask','cult',50),('Theef','cult',26),('Vangard','brother',47),('Winky','cult',8),('Xanatos','cult',50),('Xenocide','cult',50),('Xalami','cult',48),('Zaryt','snake',41),('Zigzag','cult',50),('Zasnafin','snake',13),('Zalia','cult',3),('Zashan','cult',22);
UNLOCK TABLES;
/*!40000 ALTER TABLE `factionmembers` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

