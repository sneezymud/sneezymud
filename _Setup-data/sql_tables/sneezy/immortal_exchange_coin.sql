create table immortal_exchange_coin
(
k_coin int not null AUTO_INCREMENT,
created_by int null,
created_for int null,
redeemed_by int null,
redeemed_for int null,
date_created timestamp null default CURRENT_TIMESTAMP,
date_redeemed timestamp null,
utility_flag int null,
KEY ix__immortal_exchange_coin__1 (k_coin)
);
