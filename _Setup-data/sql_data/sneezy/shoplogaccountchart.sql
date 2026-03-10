--
-- Table structure and seed data for `shoplogaccountchart`
--
-- Chart of accounts for the double-entry bookkeeping system in
-- shopaccounting.cc. Maps post_ref integers to account names used by
-- TShopJournal. Without this data, all financial statement JOINs return
-- zero rows.
--

DROP TABLE IF EXISTS `shoplogaccountchart`;
CREATE TABLE `shoplogaccountchart` (
  `post_ref` int(11) DEFAULT NULL,
  `name` text DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

LOCK TABLES `shoplogaccountchart` WRITE;
INSERT INTO `shoplogaccountchart` (post_ref, name) VALUES
(100, 'Cash'),
(101, 'Dividends'),
(130, 'Inventory'),
(300, 'Paid-in Capital'),
(310, 'Deposits'),
(500, 'Sales'),
(510, 'Recycling'),
(600, 'COGS'),
(610, 'Interest'),
(630, 'Expenses'),
(700, 'Tax'),
(800, 'Retained Earnings');
UNLOCK TABLES;
