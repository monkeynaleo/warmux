<?php

include "./net_stats_img.php";

// ========================================================= DATE

$today = date("Y-m-d");

if (array_key_exists("date", $_GET)) {
  $date = $_GET['date'];
} else {
  $date = $today;
}

// Check the $date value
$pattern='^20[0-9][0-9]\-[0-1][0-9]\-[0-3][0-9]$';
if (!ereg($pattern, $date)) {
  $date = $today;
}

// =========================================================

$hours = array();
$servers = array();
$players = array();
$max_value = 0;
$max_day = "";

function generate_stats() {
  $stats = "./net_stats/hourly.csv";
  $fh = fopen($stats, 'r');

  $i = 0;
  while (!feof($fh)) {
    $line = fgets($fh, 4096);
    $line = rtrim($line);
    $arrayline = split(" ", $line);
    $dateline = rtrim($arrayline[0]);

    if (count($arrayline) == 5) {
      if ($dateline == $GLOBALS['date']) {
	$GLOBALS['hours'][$i] = substr($arrayline[1], 0, strlen($arrayline[1]) - 3);
	if ($i % 5 == 0) {
	  $GLOBALS['hours'][$i] .= "\n".$dateline;
	}

	$GLOBALS['servers'][$i] = $arrayline[3];
	$GLOBALS['players'][$i] = $arrayline[4];
	$i++;
      }
      if ($GLOBALS['max_value'] < $arrayline[4]) {
	$GLOBALS['max_value'] = $arrayline[4];
	$GLOBALS['max_day'] = $dateline;
      }
    }
  }

  fclose($fh);
}

generate_stats();

//print $max_day;
draw_graph($hours, $players, $servers, $max_value);

?>