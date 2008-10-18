<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html lang="en" dir="ltr" xml:lang="en" xmlns="http://www.w3.org/1999/xhtml">

<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <meta name="keywords" content="wormux,wormux,wormux" />
  <meta name="robots" content="index,follow" />
  <link title="Creative Commons" type="application/rdf+xml" href="http://www.wormux.org/wiki/index.php?title=fr/index&amp;action=creativecommons" rel="meta" />
  <link rel="copyright" href="http://creativecommons.org/licenses/by-sa/2.5/" />
  <title>Translations status - Wormux</title>

<?php
  $link="http://".$_SERVER["SERVER_NAME"]."";
  $po_status = "po_status";
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

if (array_key_exists("branch", $_GET)) {
  $branch=$_GET['branch'];
} else {
  $branch = "trunk";
}

// Check the $branch value - accept only letters, numbers, '-' and '.'
$pattern='^[[:alnum:]\.\-][[:alnum:]\.\-]*$';
if (!ereg($pattern, $branch)) {
  $branch = "trunk";
}

// check the branch exists
$status_dir = "./po_status/";
if (!is_dir("$status_dir/$branch")) {
  $branch = "trunk";
}

$others_ = "";
if (is_dir($status_dir)) {
  if ($dir = opendir($status_dir)) {

    // looking for other branch
    while ($file = readdir($dir)) {

      if ($file != "." && $file != ".." && $file != $branch && is_dir("$status_dir/$file")) {
	$others_ .= "<a href=\"./po-status.php?branch=$file\">$file</a>, ";
      }
    }
  }
}
$others = rtrim($others_, ", ");
?>

<div id="texte">

<h1>
  <?php
  print "Translations status in $branch";
  ?>
</h1>

<h2>
<?php
print "Last update:";

if ($fh = fopen('./po_status/last_update', 'r')) {
  while (!feof($fh)) {
    if ($line = fgets($fh, 1024)) {
      print $line;
    }
  }
  fclose($fh);
 }

print "<br/> -- Status also available for: $others";
?>

</h2>

<div class="legend" >
  <p>
    <img src="good.png" alt="Translated color" />
    Translated
  </p>
  <p>
    <img src="fuzzy.png" alt="Fuzzy color" />
    Fuzzy translation
  </p>
  <p>
    <img src="bad.png" alt="Not translated color" />
    Not translated
  </p>
</div>

<?php

function find_languages () {
  $languages = "languages.txt";
  $ret = array();

  $fh = fopen($languages, 'r');
  while (!feof($fh)) {
    $line = fgets($fh, 4096);
    preg_match('`^(.*)=(.*)$`', $line, $buffer);
    $ret[$buffer[1]] = $buffer[2];
  }

  fclose($fh);

  return $ret;
}

function show_languages_status($branch, $lang_array) {
  if ($branch == "trunk") {
    $svn = "http://svn.gna.org/viewcvs/*checkout*/wormux/trunk/po/";
  } else {
    $svn = "http://svn.gna.org/viewcvs/*checkout*/wormux/branches/$branch/po/";
  }

  $po_dir = "./po_status/$branch";

  if (is_dir($po_dir)) {

    if ($dir = opendir($po_dir)) {

      while ($file = readdir($dir)) {
	if (preg_match('`^(.*)\.png$`', $file, $matches)) {
	  $locale = $matches[1];
	  print "<div class='postatus' >";
	  print "<p class='potext' >".$lang_array[$locale]." <span class='short' >(".$locale.")</span></p>";
	  print "<a href='".$svn.$locale.".po' ><img class='poimg' src='".$po_dir."/".$file."' alt='Translation for locale ".$locale."' /></a>";
	  print "</div>";
	}
      }
      closedir($dir);
    }
  }
}
?>

<?php
$lang_array = find_languages ();
show_languages_status($branch, $lang_array);

?>

</div>

<div id="foot">
  <p>This page is generated automatically from translation files</p>
</div>
</body></html>
