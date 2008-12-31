<?php

include "./net_stats_img.php";
include "./version_stats_img.php";

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
$disappointed_players = array();
$versions = array();
$max_value = 0;
$max_day = "";

function parse_line($i, $line)
{
  $arrayline = split(" ", $line);
  $dateline = rtrim($arrayline[0]);

  if (count($arrayline) >= 5) {

    if ($GLOBALS['max_value'] < $arrayline[4]) {
      $GLOBALS['max_value'] = $arrayline[4];
      $GLOBALS['max_day'] = $dateline;
    }

    if ($dateline == $GLOBALS['date']) {
      $GLOBALS['hours'][$i] = substr($arrayline[1], 0, strlen($arrayline[1]) - 3);
      if ($i % 5 == 0) {
	$GLOBALS['hours'][$i] .= "\n".$dateline;
      }

      $GLOBALS['servers'][$i] = $arrayline[3];
      $GLOBALS['players'][$i] = $arrayline[4];

      if (count($arrayline) > 5) {
	$GLOBALS['disappointed_players'][$i] = $arrayline[5];

	/* looking for versions... */
	$nb_versions = $arrayline[6];
	$j = 7;
	for ($k = 0; $k < $nb_versions; $k++) {
		if (array_key_exists($arrayline[$j], $GLOBALS['versions']))
			$GLOBALS['versions'][$arrayline[$j]] += $arrayline[$j+3];
		else
			$GLOBALS['versions'][$arrayline[$j]] = $arrayline[$j+3];
		$j += 5;
	}

      } else {
	$GLOBALS['disappointed_players'][$i] = 0;
      }

      return true;
    }
  }

  return false;
}

function generate_stats() {
  $stats = "./net_stats/hourly.csv";
  $fh = fopen($stats, 'r');

  $i = 0;
  while (!feof($fh)) {
    $line = fgets($fh, 4096);
    $line = rtrim($line);

    if (parse_line($i, $line))
      $i++;
  }

  fclose($fh);
}

generate_stats();

// Fill versions array
$vname = array();
$nb_players_per_version = array();

ksort($versions);
$key = key($versions);
$val = current($versions);
reset($versions);
while(list ($key, $val) = each ($versions)) {
	$vname[] = $key;
	$nb_players_per_version[] = $val;
}

// =========================================================

if (array_key_exists("max", $_GET)) {
  $max = $_GET['max'];
  if ($max > 20)
    $max_value = $max;
}

if (array_key_exists("debug", $_GET)) {
  print_arrays($players, $servers, $disappointed_players);
  print_version($vname, $nb_players_per_version);

 } else if (array_key_exists("version", $_GET)){
  draw_version_pie($vname, $nb_players_per_version);

 } else {
  draw_graph($hours, $players, $servers, $disappointed_players, $max_value);
 }

?>