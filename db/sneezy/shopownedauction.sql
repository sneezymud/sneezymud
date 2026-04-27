/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*M!100616 SET @OLD_NOTE_VERBOSITY=@@NOTE_VERBOSITY, NOTE_VERBOSITY=0 */;
DROP TABLE IF EXISTS `shopownedauction`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `shopownedauction` (
  `shop_nr` int(11) NOT NULL,
  `ticket` int(11) NOT NULL,
  `bidder` bigint(20) unsigned DEFAULT NULL,
  `buyout` int(11) DEFAULT NULL,
  `days` int(11) DEFAULT NULL,
  `current_bid` int(11) DEFAULT NULL,
  `max_bid` int(11) DEFAULT NULL,
  `seller` bigint(20) unsigned DEFAULT NULL,
  PRIMARY KEY (`ticket`),
  KEY `idx_shopownedauction_shop_nr` (`shop_nr`),
  KEY `fk_shopownedauction_seller` (`seller`),
  KEY `fk_shopownedauction_bidder` (`bidder`),
  CONSTRAINT `fk_shopownedauction_bidder` FOREIGN KEY (`bidder`) REFERENCES `player` (`id`) ON DELETE SET NULL,
  CONSTRAINT `fk_shopownedauction_seller` FOREIGN KEY (`seller`) REFERENCES `player` (`id`) ON DELETE SET NULL,
  CONSTRAINT `fk_shopownedauction_shop_nr` FOREIGN KEY (`shop_nr`) REFERENCES `shopowned` (`shop_nr`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*M!100616 SET NOTE_VERBOSITY=@OLD_NOTE_VERBOSITY */;

