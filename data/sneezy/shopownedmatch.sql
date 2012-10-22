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
-- Table structure for table `shopownedmatch`
--

DROP TABLE IF EXISTS `shopownedmatch`;
CREATE TABLE `shopownedmatch` (
  `shop_nr` int(11) default NULL,
  `match_str` varchar(128) default NULL,
  `profit_buy` double default NULL,
  `profit_sell` double default NULL,
  `max_num` int(11) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `shopownedmatch`
--


/*!40000 ALTER TABLE `shopownedmatch` DISABLE KEYS */;
LOCK TABLES `shopownedmatch` WRITE;
INSERT INTO `shopownedmatch` VALUES (35,'hammer',1.2,0.01,1),(35,'mallet',1.2,0.01,1),(117,'elven-studded-leather',0.8,0.4,1),(117,'splint-hide',0.8,0.4,1),(17,'ink',5,1,50),(113,'behir',1.2,0.6,10),(113,'sapphire',1.1,0.5,10),(113,'xorn',1,0.4,10),(35,'staff',1.2,0.01,1),(34,'black-mesh',3,1.8,10),(124,'broadsword',0.01,0.001,10),(135,'grenade',5,0.5,1000),(135,'land-mine',5,0.5,100),(34,'xorn',5,1.8,4),(34,'banded',0.2,0.01,1),(70,'vial-dragon',5,1.2,1000),(70,'blood-woman',5,1.2,1000),(70,'cheetah',5,1.2,1000),(21,'energy-drain-scroll',0.1,0.05,50),(21,'atomize-scroll',0.1,0.05,50),(70,'sky-rock',5,4.5,1000),(34,'jeweled',2,0.1,20),(34,'terbium',5,0.2,10),(113,'jeweled',1.5,0.5,10),(120,'cyan',0.5,0.2,100),(34,'sapphire',3,0.5,10),(21,'divination-scroll',1,0.5,50),(34,'orc',0.2,0.01,1),(143,'fizzling',0.46,0.3,100),(143,'sparkling-amber',0.46,0.3,100),(143,'faerie-fire',1.8,1.5,100),(143,'dark-green',0.46,0.3,100),(113,'exoskel',0,0,0),(113,'exoskeleton',0,0,0),(135,'default',1,0.5,5),(135,'death-camas-extract',2,0.7,100),(135,'destroying-angel-extract',2,0.7,100),(135,'jimson-weed-extract',2,0.7,100),(113,'molten',1.3,1,10),(135,'hemlock-extract',2,0.7,100),(40,'gleaming',2,1,10),(143,'crumpled-haste',0.95,0.8,100),(143,'energy-drain',0.4,0.14,50),(143,'atomize',0.3,0.2,100),(143,'meteor-swarm',0.1,0.01,50),(143,'blast-fury',0.1,0.01,50),(117,'frogskin',0.8,0.4,1),(117,'sheepskin',0.8,0.4,1),(17,'comp',3,0.2,51),(21,'true-sight-scroll',0.5,0.25,50),(143,'icy-grip',1.7,1,100),(20,'epee',5,1,10),(34,'ringmail',0.2,0.01,1),(34,'platemail',0.2,0.01,1),(35,'arrow',5,0.01,1),(35,'quarrel',5,0.01,1),(34,'gnom',0.2,0.01,1),(35,'train',0.2,0.01,1),(35,'wood',0.2,0.01,1),(35,'spear',1.2,0.01,1),(35,'club',1.2,0.01,1),(143,'rusty',0.46,0.3,100),(113,'ring-jewel',1.5,0.5,10),(226,'faerie-fire-scroll',3.3,0.1,1000),(226,'atomize-scroll',0.3,0.1,1000),(226,'icy-grip-scroll',2.2,0.1,1000),(226,'stone-skin-scroll',1.4,0.1,1000),(226,'energy-drain-scroll',0.3,0.1,1000),(226,'fear-scroll',1.1,0.1,1000),(226,'teleport-scroll',2.7,0.1,1000),(226,'invisibility-scroll',1.4,0.1,1000),(226,'stealth-scroll',2.2,0.1,1000),(226,'haste-scroll',1.5,0.1,1000),(226,'sorcerers-globe-scroll',4.4,0.1,1000),(226,'falcon-wings-scroll',0.9,0.1,1000),(226,'gills-flesh-scroll',5,0.1,1000),(226,'deep-blue-scroll',1.1,0.1,1000),(226,'golden-potion',1.6,0.1,1000),(226,'light-blue-potion',1.1,0.1,1000),(226,'white-potion',1.1,0.1,1000),(226,'silver-potion',1.1,0.1,1000),(226,'cyan-potion',1.5,0.1,1000),(226,'black-potion',1.1,0.1,1000),(226,'platinum-potion',0.8,0.1,1000),(226,'gray-potion',1.1,0.1,1000),(226,'granite-colour-potion',1.1,0.1,1000),(226,'crimson-potion',4,0.1,1000),(226,'protection-from-fire',4.4,0.1,1000),(226,'protection-from-water',4.4,0.1,1000),(226,'protection-from-earth',4.4,0.1,1000),(226,'protection-from-element',4.4,0.1,1000),(226,'protection-from-air',4.4,0.1,1000),(122,'card',1.5,0.5,3),(226,'sturdy-glass-flask',5,0.1,1000),(226,'dispel-magic-scroll',2,0.1,1000),(226,'calm-scroll',1.6,0.1,1000),(226,'detect-invis-scroll',3.8,0.1,1000),(226,'levitate-scroll',2.2,0.1,1000),(226,'pale-white-potion',1,0.1,1000),(113,'mica-greave',0.9,0.4,10),(226,'rusty-potion',1,0.1,1000),(226,'torn-scroll',3.3,0.1,1000),(22,'walnut-arrow',0.5,0.1,500),(22,'teak-arrow',0.5,0.1,500),(22,'oak-arrow',0.5,0.1,500),(22,'poplar-arrow',0.5,0.1,500),(22,'cedar-arrow',0.5,0.1,500),(22,'balsam-arrow',0.5,0.1,500),(22,'mahogany-arrow',0.5,0.1,500),(22,'hickory-arrow',0.5,0.1,500),(226,'steel-grey-potion',1,0.1,1000),(226,'shad-crim-potion',3,0.1,1000),(143,'invisibil',1,0.7,50),(143,'det-invisibil',3,1.6,50),(143,'haste',0.8,0.7,50),(143,'true-sight',0.7,0.4,50),(143,'sorcerers-globe',3,1.6,50),(143,'icey-grip',1.8,1.5,1000),(143,'pale-white',0.46,0.3,100),(143,'orange-potion',2,0.7,50),(143,'silver-potion',1,0.7,50),(143,'slate-grey',0.46,0.3,50),(143,'steel-grey',0.46,0.3,50),(143,'protection-from-fire',5,3.5,100),(143,'crumpled-stealth',2,1.3,50),(143,'dispel-magic',1.6,0.9,50),(226,'fizzling-potion',1,0.1,1000),(226,'dark-green-potion',1,0.1,1000),(226,'sparkling-amber-potion',1,0.1,1000),(226,'slate-grey-potion',1,0.1,1000),(22,'bow',3,0.1,100),(22,'quiver',2,0.1,10),(143,'flaming-sword',1.5,0.9,50),(172,'bronze',1.5,0.001,0),(226,'scarlet-potion',1,0.1,1000),(172,'rusty',0.356,0.0001,300),(172,'fizzling',0.001,0.0001,300),(172,'fizz',0.356,0.0001,300),(172,'bright-golden',1.5,0.0001,100),(172,'green-potion',0.25,0.0001,100),(172,'steel-potion',0.8,1e-06,50),(172,'slate-potion',0.8,1e-05,50),(172,'faerie-fire',2.9,0.0001,100),(172,'rusty-potion',0.356,0.0001,300),(172,'fizz-pot',0.001,0.0001,300),(226,'rusty-potion;wp',1.5,0.1,2000),(226,'fizzling-potion;wp',1.5,0.1,1000),(226,'sparkling-amber-potion;wp',1.5,0.1,1000),(226,'true-scroll',1.5,0.1,1000),(29,'copper-scroll',0.3,0.1,10),(29,'atomize-scroll',0.4,0.1,20),(29,'prot-from-fire',5,0.1,20),(29,'scroll',2,0.1,20),(29,'faerie',3.75,0.1,20),(29,'granite-scroll',1.75,0.1,20),(29,'swamp',2,0.1,100),(29,'foxglove',1.5,0.1,100),(29,'tongue',1.5,0.1,100),(29,'flower',2,0.1,100),(29,'rose',2,0.1,100),(29,'fly',5,0.1,100),(29,'sparkling',0.55,0.1,100),(29,'pale-white',0.55,0.1,100),(29,'steel-grey',0.4,0.1,100),(29,'slate-grey',0.4,0.1,100),(29,'dark-green',0.5,0.1,100),(29,'fizzling',0.6,0.1,20),(258,'spellbag',1.1,0.9,1),(258,'grenulpa',1.1,0.9,1),(121,'emerald',3,0.0001,10),(121,'mithril',0.75,1e-06,10),(121,'large-vial',0.3,1e-06,30),(121,'holy-vial',0.3,0,30),(121,'gold-symbol',1,1e-06,10),(121,'gold-sy',1.1,1e-06,10),(121,'marble-symbol',2.5,1e-06,10),(121,'marb-sy',3,0,10),(173,'breeches',5,1e-06,10),(173,'kuo',5,1e-05,10),(173,'luna',5,0.0001,10);
UNLOCK TABLES;
/*!40000 ALTER TABLE `shopownedmatch` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

