<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html lang="en" dir="ltr" xml:lang="en" xmlns="http://www.w3.org/1999/xhtml">

<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <meta name="keywords" content="wormux,wormux,wormux" />
  <meta name="robots" content="index,follow" />
  <link title="Creative Commons" type="application/rdf+xml" href="http://www.wormux.org/wiki/index.php?title=fr/index&amp;action=creativecommons" rel="meta" />
  <link rel="copyright" href="http://creativecommons.org/licenses/by-sa/2.5/" />
  <title>Network game statistics  - Wormux</title>

   <script type="text/javascript" src="calendarDateInput.js">

   /***********************************************
    * Jason's Date Input Calendar- By Jason Moon http://calendar.moonscript.com/dateinput.cfm
    * Script featured on and available at http://www.dynamicdrive.com
    * Keep this notice intact for use.
    ***********************************************/
   </script>

<?php
  $link="http://".$_SERVER["SERVER_NAME"]."";
?>
<style>
		#entete { background: url('<?php echo $link; ?>/entete/entete.png') repeat-x; float: left; border: 1px solid #dedede; border-bottom:0pt; margin-left:15px; width: 98%; max-width: 98%; min-width: 98%; <?php if (ereg("MSIE", getenv("HTTP_USER_AGENT"))) echo "margin-top: -10px; margin-bottom: -18px;"; else echo "margin-top: 12px; margin-bottom: 0;" ?>}
		#ylogo { border:0; align: left; position: absolute; margin-bottom: 0pt; margin-top: 0pt;}
		#ylink { margin-left:0px; margin-bottom:0px; clear: both; position: relative; font-size: 0.7em; padding-left: 0pt; <?php if (ereg("MSIE", getenv("HTTP_USER_AGENT"))) echo "padding-top: 0px;"; else echo "padding-top: 20px;" ?> list-style-type: none;}
		#ylink li { background: url('<?php echo $link; ?>/entete/header_tab.gif') 100% -600px; padding: 0pt 3px 0pt 0pt; float: left; margin: 0pt 1px 0pt 0pt;}
		#ylink li:hover { background-position: 100% -400px;}
		#ylink li a { padding: 4px 4px 4px 4px; background: url('<?php echo $link; ?>/entete/header_tab.gif') 0% -600px; float: left; display: block; text-decoration: none; color: #000000;}
		#ylink li:hover a { background-position: 0% -400px; color: rgb(0, 51, 204);}
		#cherch { padding: 12px 20px 0pt 0pt; position: absolute; top: 0pt; right: 0pt; text-align: right;}
		#dd {  border:0; position: relative; float: right; margin-top: 50px; margin-right:10px; }
		#sear {background: #dedede; width: 150px; border: 1px solid #ffffff; vertical-align: middle;}
		#yflags { position: relative; float: right; padding-top:5px; padding-right:5px;}
		#yflags img { border:0;}
		#texte {border : 1px solid #dedede; border-top: 0px; margin-top: -1px; margin-left:15px; background : #ffffff; float: left; width: 98%; max-width: 98%; min-width: 98%; font-size: 0.8em;}
		#texte h1 { font-size: 1.6em; font-weight: bolder; color: #d4290b; margin-top: 10pt; margin-left: 20pt; border: 0;}
		#texte h2 { font-size: 1.2em; color: #000000; margin-top: 15pt; margin-left: 30pt; border: 0; border-bottom: 1px solid #aaaaaa;}
		#texte ul { margin-left: 40pt;}
		#foot {background: url('foot.png') repeat-x; border : 1px solid #dedede; border-top:0; margin-top: -1px; margin-left:15px; float: left; width: 98%; max-width: 98%; min-width: 98%; font-size: 0.8em; color:#ffffff; text-align: center; margin-bottom:20px;}
		#foot a {color:#ffffff;}

div.postatus {
  width: 650px;
  height: 3em;
  margin: 0 auto 0 auto;
}

.potext {
  font-weight: bold;
  margin: .5em 0 .5em 0;
  display: inline;
  float: left;
}

.potext .short {
  font-weight: normal;
}

.poimg {
  float: right;
  padding: .5em 0 .5em 0;
}

a img {
  border: none;
  }

a:active .poimg, a:hover .poimg {
  background-color: #dedede;
  }

.legend {
  width: 600px;
  margin: 1em auto 1em auto;
  padding-left: 200px;
}

.legend p {
  display: inline;
}

 	</style>
</head>

<body style="background: #d2c38c; min-width: 770px; font-family: verdana,sans-serif;" >

<?php include ("../entete/index.php");

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

?>

<div id="texte">

<h1>Network game statistics</h1>

<p>See below connections statistics to the index server.</p>

<p><strong>Those measures don't show (yet) the real number of players !!</strong></p>

<p>Those 2 graphs show the number of connections opened to the server and not the number games actually started.</p>

<ul>
<li>The <font color="blue">blue curve</font> shows the number of game hosted.</li>
<li>The <font color="red">red curve</font> shows the number of game hosted plus the number of people that (wanted to) join(ed) a game.</li>
</ul>

<h2>Connections per hour done during day <?php print $date ?></h2>

<form action="./network-stats.php" method="get">
  <script>DateInput('date', true, 'YYYY-MM-DD', '<?php print $date ?>')</script>
  <input type="submit" value="Update">
</form>

<p>
<img src='net_hourly.php?date=<?php print $date ?>' border=1 bordercolor=black/>
</p>

<h2>Connections done daily</h2>

<p>
<img src='net_daily.php' border=1 bordercolor=black/>
</p>

</div>

<div id="foot">
  <p>This page is generated automatically from network index server statistics</p>
</div>

</body>
</html>
