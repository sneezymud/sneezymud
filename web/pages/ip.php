<?php
header ("Content-type: image/jpeg");
//dl("extensions/php_gd.dll");
$im = ImageCreate (500, 200)
    or die ("Cannot Initialize new GD image stream");
$background = ImageColorAllocate ($im, 0, 0, 0);
$textcolor = ImageColorAllocate ($im, 128, 128, 128);
$highcolor = ImageColorAllocate ($im, 25, 155, 25);
$lowcolor = ImageColorAllocate ($im, 155, 25, 25);
$averagecolor = ImageColorAllocate ($im, 100, 100, 255);


imagestring ($im, 5, 250, 100, "Your IP: ".getenv ("REMOTE_ADDR"), $textcolor);

imagejpeg($im);
imageDestroy($im);

?>
