<?php

include "./net_stats_img.php";

// =========================================================

$begindate = "";
if (array_key_exists("begin", $_GET)) {
  $begindate = $_GET['begin'];
}

$enddate="";
if (array_key_exists("end", $_GET)) {
  $enddate = $_GET['end'];
}

// =========================================================

$days = array();
$servers = array();
$players = array();

function generate_stats() {
  $stats = "./net_stats/daily.csv";
  $fh = fopen($stats, 'r');

  $beginline = split("-", $GLOBALS['begindate']);
  $endline = split("-", $GLOBALS['enddate']);

  $i = 0;
  while (!feof($fh)) {
    $line = fgets($fh, 4096);
    $line = rtrim($line);
    $arrayline = split(" ", $line);

    if (count($arrayline) == 5) {

      $date = $arrayline[0];
      $dateline = split("-", $date);

      if (count($beginline) == 3) {
	if ($dateline[0] < $beginline[0]) // year
	  continue;

	if ($dateline[0] == $beginline[0]
	    && $dateline[1] < $beginline[1]) // month
	  continue;

	if ($dateline[0] == $beginline[0]
	    && $dateline[1] == $beginline[1]
	    && $dateline[2] < $beginline[2]) // day
	  continue;
      }

      if (count($endline) == 3) {
	if ($dateline[0] > $endline[0]) // year
	  continue;

	if ($dateline[0] == $endline[0]
	    && $dateline[1] > $endline[1]) // month
	  continue;

	if ($dateline[0] == $endline[0]
	    && $dateline[1] == $endline[1]
	    && $dateline[2] > $endline[2])
	  continue;
      }

      $GLOBALS['days'][$i] = $arrayline[0];
      $GLOBALS['servers'][$i] = $arrayline[3];
      $GLOBALS['players'][$i] = $arrayline[4];
      $i++;
    }
  }
  fclose($fh);
}

generate_stats();

$max = max($players);

draw_graph($days, $players, $servers, $max);

?>