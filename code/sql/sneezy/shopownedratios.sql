CREATE TABLE shopownedratios (
    shop_nr integer DEFAULT '0' NOT NULL,
    obj_nr integer DEFAULT '0' NOT NULL,
    profit_buy double precision,
    profit_sell double precision,
    max_num integer
);
