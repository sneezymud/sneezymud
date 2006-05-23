<?

if (!isset($word)) {
  $word = "test, pHRase";
}

$word = strtolower($word);
$word = str_replace(",", "", $word);
$word = str_replace(" ", "", $word);


$wordcount = (count_chars($word, 0));

$fd = fopen ("/usr/share/dict/words", "r");
while (!feof ($fd)) {
    $OK = 1;
    $buffer = fgets($fd, 4096);

    $buffer = trim(strtolower($buffer));
    $guesscount = (count_chars($buffer, 0));

    for ($i=0; $i < (sizeof($wordcount)); $i++){
        if ($wordcount[$i] < $guesscount[$i])
                $OK = 0;
    }

    if($OK == 1)
        echo "".$buffer.", ";

}
fclose ($fd);

?>
