create table shoplogjournalarchive (
  shop_nr int,
  journal_id int auto_increment,
  customer_name text,
  obj_name text,
  sneezy_year int,
  logtime timestamp,
  post_ref int,
  debit int,
  credit int,
  key (journal_id)
);

