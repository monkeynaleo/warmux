<?php

require_once "Artichow/LinePlot.class.php";

function draw_graph($legends, $players_array, $servers_array, $disappointed_players_array, $maxy)
{
  $graph = new Graph();
  $graph->setTiming(FALSE);
  $graph->setSize(640, 480);
  $graph->setAntiAliasing(FALSE);

  $group = new PlotGroup();
  $group->setPadding(40, 40);
  $group->SetYMax($maxy);
  $group->grid->setType(LINE_DOTTED);
  $group->grid->setInterval(1, 2);

  // Compute interval
  $size = count($legends);
  if ($size <= 8) {
    $interval = 1;
  } elseif ($size <= 16) {
    $interval = 2;
  } else {
    $interval = $size/5;
  }

  $group->axis->bottom->setLabelInterval($interval);
  $group->axis->bottom->setLabelText($legends);
  $group->axis->bottom->setNumberByTick('minor', 'major', $interval);

  // Prepare the players graph
  $players_plot = new LinePlot($players_array);

  $players_plot->setColor(new Red);
  $players_plot->setThickness(1);

  // Prepare the servers graph
  $servers_plot = new LinePlot($servers_array);

  $servers_plot->setColor(new Blue);
  $servers_plot->setThickness(1);

  // Prepare the disappointed graph
  $disappointed_plot = new LinePlot($disappointed_players_array);

  $disappointed_plot->setColor(new Green);
  $disappointed_plot->setThickness(1);

  // Time's to draw!!
  $group->add($players_plot);
  $group->legend->add($players_plot, "players (servers + clients)");

  $group->add($servers_plot);
  $group->legend->add($servers_plot, "games (servers)");

  $group->add($disappointed_plot);
  $group->legend->add($disappointed_plot, "disappointed clients");

  $graph->add($group);
  $graph->draw();
}


function print_arrays($players, $servers, $disappointed)
{
  print "<table border=\"1\">\n";

  print "<tr>\n";
  $i = 0;
  print "<td>".count($GLOBALS['hours'])." hours: </td>\n";
  while ($i < count($GLOBALS['hours'])) {
    print "<td>".$GLOBALS['hours'][$i]."</td>\n";
    $i++;
  }
  print "</tr>\n";

  print "<tr>\n";
  $i = 0;
  print "<td>".count($players)." players: </td>\n";
  while ($i < count($players)) {
    print "<td>".$players[$i]."</td>\n";
    $i++;
  }
  print "</tr>\n";

  print "<tr>\n";
  $i = 0;
  print "<td>".count($disappointed)." disappointed clients: </td>\n";
  while ($i < count($disappointed)) {
    print "<td>".$disappointed[$i]."</td>\n";
    $i++;
  }

  print "<tr>\n";
  $i = 0;
  print "<td>".count($servers)." servers: </td>\n";
  while ($i < count($servers)) {
    print "<td>".$servers[$i]."</td>\n";
    $i++;
  }
  print "</tr>\n";

  print "</table>\n";
}

?>