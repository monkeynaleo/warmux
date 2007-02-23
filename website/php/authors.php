<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html lang="en" dir="ltr" xml:lang="en" xmlns="http://www.w3.org/1999/xhtml">

<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <meta name="keywords" content="wormux,wormux,wormux" />
  <meta name="robots" content="index,follow" />
  <link title="Creative Commons" type="application/rdf+xml" href="http://www.wormux.org/wiki/index.php?title=fr/index&amp;action=creativecommons" rel="meta" />
  <link rel="copyright" href="http://creativecommons.org/licenses/by-sa/2.5/" />
  <link rel="stylesheet" href="authors.css" />
  <title>Authors - Wormux</title>
  
<?php
  $link="http://".$_SERVER["SERVER_NAME"]."";
  $svnat="http://svn.gna.org/viewcvs/*checkout*/wormux/trunk/data/authors.xml";
?>
<style>
		#entete { background: url('<?php echo $link; ?>/entete/entete.png') repeat-x; float: left; border: 1px solid #dedede; border-bottom:0pt; margin-left:15px; width: 97.5%; max-width: 97.5%; min-width: 97.5%; margin-top: 12px; margin-bottom: 0;}
		#ylogo { border:0; align: left; position: absolute; margin-bottom: 0pt; margin-top: 0pt;}
		#ylink { clear: both; position: relative; font-size: small; padding-left: 0pt; padding-top: 20px; list-style-type: none; font-weight: bold; }
		#ylink li { background: url('<?php echo $link; ?>/entete/header_tab.gif') 100% -600px; padding: 0pt 3px 0pt 0pt; float: left; margin: 0pt 1px 0pt 0pt;}
		#ylink li:hover { background-position: 100% -400px;}
		#ylink li a { padding: 4px 4px 4px 10px; background: url('<?php echo $link; ?>/entete/header_tab.gif') 0% -600px; float: left; display: block; text-decoration: none; color: #000000;}
		#ylink li:hover a { background-position: 0% -400px; color: rgb(0, 51, 204);}
		#cherch { padding: 12px 20px 0pt 0pt; position: absolute; top: 0pt; right: 0pt; text-align: right;}
		#dd {  border:0; position: relative; float: right; margin-top: 50px; margin-right:10px; }
		#sear {background: #dedede; width: 150px; border: 1px solid #ffffff; vertical-align: middle;}
		#yflags { position: relative; float: right; padding-top:5px; padding-right:5px;}
		#yflags img { border:0;}
		#texte {border : 1px solid #dedede; border-top: 0px; margin-top: -1px; margin-left:15px; background : #ffffff; float: left; width: 97.5%; max-width: 97.5%; min-width: 97.5%; font-size: 0.8em;}
		#texte h1 { font-size: 1.6em; font-weight: bolder; color: #d4290b; margin-top: 10pt; margin-left: 20pt; border: 0;}
		#texte h2 { font-size: 1.2em; color: #000000; margin-top: 15pt; margin-left: 30pt; border: 0; border-bottom: 1px solid #aaaaaa;}
		#texte ul { margin-left: 40pt;}
		#foot {background: url('foot.png') repeat-x; border : 1px solid #dedede; border-top:0; margin-top: -1px; margin-left:15px; float: left; width: 97.5%; max-width: 97.5%; min-width: 97.5%; font-size: 0.8em; color:#ffffff; text-align: center; margin-bottom:20px;}
		#foot a {color:#ffffff;}
	</style>
</head>

<body style="background: #d2c38c; min-width: 770px; font-family: verdana,sans-serif;";>

<?php include ("../entete/index.php"); ?>

<div id="texte" >

<?php

$xml_file = $svnat;
$xsl_file = $link.'/php/authors.xsl';

// Load files
$DomXmlFile = domxml_open_file($xml_file);
$DomXslTrans = domxml_xslt_stylesheet_file($xsl_file);

// Apply stylesheet
$DomXmlFileResult=$DomXslTrans->process($DomXmlFile,$PARAM);
$XmlFileResult= $DomXmlFileResult->dump_mem(true);

// Print result
print $XmlFileResult;

?>

</div>

<div id="foot" >
  <p>This page is generated automatically from :<br />
    <a href="<?php echo $svnat;?>"><?php echo $svnat;?></a>.
  </p>
</div>
</body>
</html>
