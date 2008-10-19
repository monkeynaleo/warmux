<?php

require_once "Artichow/LinePlot.class.php";

function draw_graph($legends, $players_array, $servers_array, $maxy)
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

  // Time's to draw!!
  $group->add($players_plot);
  $group->legend->add($players_plot, "players");

  $group->add($servers_plot);
  $group->legend->add($servers_plot, "games (servers)");

  $graph->add($group);
  $graph->draw();
}

?>