myslcreate table shoplogjournal (
  shop_nr int,
  journal_id int auto_increment,
  customer_name text,
  obj_name text,
  logtime timestamp,
  post_ref int,
  debit int,
  credit int,
  key (journal_id)
);

