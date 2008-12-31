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
$disappointed_players = array();

function parse_line($i, $line, $beginline, $endline)
{
  $arrayline = split(" ", $line);

  if (count($arrayline) >= 5) {

    $date = $arrayline[0];
    $dateline = split("-", $date);

    if (count($beginline) == 3) {
      if ($dateline[0] < $beginline[0]) // year
	return false;

      if ($dateline[0] == $beginline[0]
	  && $dateline[1] < $beginline[1]) // month
	return false;

      if ($dateline[0] == $beginline[0]
	  && $dateline[1] == $beginline[1]
	  && $dateline[2] < $beginline[2]) // day
	return false;
    }

    if (count($endline) == 3) {
      if ($dateline[0] > $endline[0]) // year
	return false;

	if ($dateline[0] == $endline[0]
	    && $dateline[1] > $endline[1]) // month
	  return false;

	if ($dateline[0] == $endline[0]
	    && $dateline[1] == $endline[1]
	    && $dateline[2] > $endline[2])
	  return false;
    }

    $GLOBALS['days'][$i] = $arrayline[0];
    $GLOBALS['servers'][$i] = $arrayline[3];
    $GLOBALS['players'][$i] = $arrayline[4];

    if (count($arrayline) > 5) {
      $GLOBALS['disappointed_players'][$i] = $arrayline[5];
    } else {
      $GLOBALS['disappointed_players'][$i] = 0;
    }

    return true;
  }

  return false;
}

function generate_stats() {
  $stats = "./net_stats/daily.csv";
  $fh = fopen($stats, 'r');

  $beginline = split("-", $GLOBALS['begindate']);
  $endline = split("-", $GLOBALS['enddate']);

  $i = 0;
  while (!feof($fh)) {
    $line = fgets($fh, 4096);
    $line = rtrim($line);

    if (parse_line($i, $line, $beginline, $endline))
      $i++;
  }
  fclose($fh);
}

generate_stats();

$max = max($players);

if (array_key_exists("debug", $_GET)) {
  print_arrays($players, $servers, $disappointed_players);
 } else {
  draw_graph($days, $players, $servers, $disappointed_players, $max);
 }

?>