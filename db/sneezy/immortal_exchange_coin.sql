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
DROP TABLE IF EXISTS `immortal_exchange_coin`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `immortal_exchange_coin` (
  `k_coin` int(11) NOT NULL AUTO_INCREMENT,
  `created_by` bigint(20) unsigned DEFAULT NULL,
  `created_for` bigint(20) unsigned DEFAULT NULL,
  `redeemed_by` bigint(20) unsigned DEFAULT NULL,
  `redeemed_for` bigint(20) unsigned DEFAULT NULL,
  `date_created` timestamp NULL DEFAULT current_timestamp(),
  `date_redeemed` timestamp NULL DEFAULT NULL,
  `utility_flag` int(11) DEFAULT NULL,
  `created_by_name` varchar(80) DEFAULT NULL,
  `created_for_name` varchar(80) DEFAULT NULL,
  `redeemed_by_name` varchar(80) DEFAULT NULL,
  `redeemed_for_name` varchar(80) DEFAULT NULL,
  PRIMARY KEY (`k_coin`),
  KEY `idx_iec_created_by` (`created_by`),
  KEY `idx_iec_created_for` (`created_for`),
  KEY `idx_iec_redeemed_by` (`redeemed_by`),
  KEY `idx_iec_redeemed_for` (`redeemed_for`),
  CONSTRAINT `fk_immortal_exchange_coin_created_by` FOREIGN KEY (`created_by`) REFERENCES `player` (`id`) ON DELETE SET NULL,
  CONSTRAINT `fk_immortal_exchange_coin_created_for` FOREIGN KEY (`created_for`) REFERENCES `player` (`id`) ON DELETE SET NULL,
  CONSTRAINT `fk_immortal_exchange_coin_redeemed_by` FOREIGN KEY (`redeemed_by`) REFERENCES `player` (`id`) ON DELETE SET NULL,
  CONSTRAINT `fk_immortal_exchange_coin_redeemed_for` FOREIGN KEY (`redeemed_for`) REFERENCES `player` (`id`) ON DELETE SET NULL
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

