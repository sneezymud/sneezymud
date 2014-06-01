CREATE TABLE factorysupplies (
  shop_nr int,
  supplytype int,
  supplyname varchar(16),
  supplyamt int  
);

CREATE TABLE factoryproducing (
  shop_nr int,
  vnum int
);

CREATE TABLE factoryblueprint (
  vnum int,
  supplytype int,
  supplyamt int
);

