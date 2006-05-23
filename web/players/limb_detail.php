<?
// TEAM DEFINITION
$NUM_TEAMS = 2;
$teams = array();
$teams[1] = array('Beloc', 'Peewee', 'Crazed', 'Merkaba', 'Sephie', 'Shakudo',
  'Thelonious', 'Baffleflap', 'Raven', 'Xalami', 'Fitz', 'Releep', 'Ashcroft', 'Blum', 'Weps', 'Jiraiya',
  'Antonius');
$teams[2] = array('Apollyon', 'Laren', 'Slab', 'Arkane', 'IRS', 'Selch', 'Ferry',
  'Intuition', 'Frobozz', 'Stin', 'Legato', 'Trina', 'Aiolos', 'Fedaykin', 'Kelranth',
  'Trask', 'Amberbock');

// limb array
$limbs = array();
$limbs[0] = 'bogus';
$limbs[1] = 'head of';
$limbs[2] = 'bloody eyeballs';
$limbs[3] = 'lifeless heart of';
$limbs[4] = 'right arm of';
$limbs[5] = 'left arm of';
$limbs[6] = 'right hand of';
$limbs[7] = 'left hand of';
$limbs[8] = 'right finger of';
$limbs[9] = 'left finger of';
$limbs[10] = 'genitalia of';
$limbs[11] = 'right foot of';
$limbs[12] = 'left foot of';
$limbs[13] = 'bloody tooth of';

// set our modes
if ($mode < 1 OR $mode > 13) $mode = 0;
if ($team < 1 OR $team > $NUM_TEAMS) $team = 0;

// read the logs
// format roughly:
// <date string> :: Maror: <limb string> chopped by <team member a>, vnum <vnum>, deposited by <team member b> at <date string>
system("/mud/web/limbfest/limb_log_gen.py", $returnval);
$dir = "/mud/web/limbfest/logs/";
$dh  = opendir($dir);
while (false !== ($filename = readdir($dh))) {
   $filelist[] = $filename;
}
rsort($filelist);
$choppedlist = array();

foreach ($filelist as $filename) {
	$loglines = file($dir."/".$filename);
	foreach($loglines as $line) {
		$data = explode("Maror: ", trim($line));
		$data = explode(", ", $data[1]);
		$chopstr = explode(" chopped by ", $data[0]);
		$limbstr = str_replace("bloody, mangled", "", $chopstr[0]);
		$chopper = $chopstr[1];
		$vnum = str_replace("vnum ", "", $data[1]);
		$team = 0;
		foreach ($teams as $teamnum => $roster) {
			if (in_array($chopper, $roster)) {
				$team = $teamnum;
			}
		}
		foreach ($limbs as $limbnum => $limbtext) {
			if (strpos($limbstr, $limbtext)) $whichlimb = $limbnum;
		}
	$choppedlist[] = array('team' => $teamnum, 'limb' => $limbnum, 'vnum' => $vnum,
				'choptext' => $limbstr, 'chopper' => $chopper);
	}
	if ($filename == "log.062304.2142") break;
}


?>
<HTML>
<HEAD>
<TITLE>Limbfest June 2004 Detail Page</TITLE>
</HEAD>
<BODY>



</BODY>
</HTML>