<?php

include "./net_stats_img.php";

$days = array();
$servers = array();
$players = array();

function generate_stats() {
  $stats = "./net_stats/daily.csv";
  $fh = fopen($stats, 'r');

  $i = 0;
  while (!feof($fh)) {
    $line = fgets($fh, 4096);
    $line = rtrim($line);
    $arrayline = split(" ", $line);

    if (count($arrayline) == 5) {
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