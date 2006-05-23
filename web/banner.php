<?

$host = getenv("REMOTE_ADDR");

$fp = fopen("dash/hostlog","a");
if ($fp) {
   fputs($fp, time()."   ".$host."<br>");
   fclose($fp);
}


Header("Content-type: image/jpeg");
$im = imagecreatefromjpeg("/mud/web/dash/corsairs.jpg");
Imagejpeg($im,'',75);
ImageDestroy($im);
?>
