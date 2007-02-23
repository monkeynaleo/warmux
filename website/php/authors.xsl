<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" indent="yes"/>
  <xsl:param name="generated-for"></xsl:param>
  
  <xsl:template name="oddeven">
    <xsl:choose>
      <xsl:when test="position() mod 2 = 0" ><xsl:text>odd</xsl:text></xsl:when>
      <xsl:otherwise><xsl:text>even</xsl:text></xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="section">
    <xsl:for-each select="section" >
      <h2>
	<xsl:attribute name="class"><xsl:text>section</xsl:text></xsl:attribute>
	<xsl:value-of select="@title" />
      </h2>
      <ul>
	<xsl:attribute name="class"><xsl:text>author</xsl:text></xsl:attribute>
	<xsl:for-each select="author" >
	  <li>
	    <xsl:attribute name="class" >
	      <xsl:call-template name="oddeven" />
	    </xsl:attribute>
	    <span>
	      <xsl:attribute name="class"><xsl:text>nickname</xsl:text></xsl:attribute>
	      <xsl:value-of select="nickname" />
	    </span>
	    <xsl:if test="name" >
	      <span>
		<xsl:attribute name="class"><xsl:text>authorname</xsl:text></xsl:attribute>
		( <xsl:value-of select="name" /> )
	      </span>
	    </xsl:if>
	    :
	    <span>
	      <xsl:attribute name="class"><xsl:text>authordescription</xsl:text></xsl:attribute>
	      <xsl:value-of select="description" />
	    </span>
	    <span>
	      <xsl:attribute name="class"><xsl:text>authorlinks</xsl:text></xsl:attribute>
	      <xsl:if test="website" >
		<a>
		  <xsl:attribute name="class"><xsl:text>authorwebsite</xsl:text></xsl:attribute>
		  <xsl:attribute name="href"><xsl:value-of select="website" /></xsl:attribute>
		  <xsl:attribute name="title">
		    <xsl:text>Go to </xsl:text>
		    <xsl:value-of select="nickname" />
		    <xsl:text>'s website</xsl:text>
		  </xsl:attribute>
		  <img>
		    <xsl:attribute name="src"><xsl:text>icons/www.png</xsl:text></xsl:attribute>
		    <xsl:attribute name="alt"><xsl:text>Web link icon</xsl:text></xsl:attribute>
		  </img>
		</a>
	      </xsl:if>
	      <xsl:if test="email" >
		<a>
		  <xsl:attribute name="class"><xsl:text>authormail</xsl:text></xsl:attribute>
		  <xsl:attribute name="href">
		    <xsl:text>mailto:</xsl:text>
		    <xsl:value-of select="email" />
		  </xsl:attribute>
		  <xsl:attribute name="title">
		    <xsl:text>Contact </xsl:text>
		    <xsl:value-of select="nickname" />
		  </xsl:attribute>
		  <img>
		    <xsl:attribute name="src"><xsl:text>icons/mail.png</xsl:text></xsl:attribute>
		    <xsl:attribute name="alt"><xsl:text>Email icon</xsl:text></xsl:attribute>
		  </img>
		</a>
	      </xsl:if>
	      <xsl:if test="country" >
		<img>
		  <xsl:attribute name="class"><xsl:text>flag</xsl:text></xsl:attribute>
		  <xsl:attribute name="src">
		    <xsl:text>icons/</xsl:text><xsl:value-of select="country" /><xsl:text>.png</xsl:text>
		  </xsl:attribute>
		  <xsl:attribute name="alt">
		    <xsl:value-of select="country" />
		    <xsl:text>'s flag</xsl:text>
		  </xsl:attribute>
		  <xsl:attribute name="title">
		    <xsl:value-of select="country" />
		  </xsl:attribute>
		</img>
	      </xsl:if>
	    </span>
	  </li>
	</xsl:for-each>
      </ul>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="/authors" >

    <xsl:for-each select="team" >
      <h1>
	<xsl:text>Team</xsl:text>
      </h1>
      <xsl:call-template name="section" />
    </xsl:for-each>

    <xsl:for-each select="contributors" >
      <h1>
	<xsl:text>Contributors</xsl:text>
      </h1>
      <xsl:call-template name="section" />
    </xsl:for-each>

    <xsl:for-each select="thanks" >
      <h1>
	<xsl:text>Thanks</xsl:text>
      </h1>
      <xsl:call-template name="section" />
    </xsl:for-each>

  </xsl:template>

</xsl:stylesheet>