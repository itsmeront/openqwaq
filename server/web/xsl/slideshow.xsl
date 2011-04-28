<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE xsl:stylesheet [
	  <!ENTITY nbsp "&#160;">
	  <!ENTITY copy "&#169;">
]>
<xsl:stylesheet version="2.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		>
  <xsl:template match="slide">
    <img>
      <xsl:attribute name="src">
	<xsl:text>/download/</xsl:text>
	<xsl:value-of select="ref"/>
	<xsl:text>?organization=</xsl:text>
	<xsl:value-of select="/fp/org/ref"/>
	<xsl:text>&amp;forum=</xsl:text>
	<xsl:value-of select="/fp/forum/ref"/>
      </xsl:attribute>
    </img>
  </xsl:template>

  <xsl:template match="/">
    <html>
      <head>
	<title><xsl:value-of select="fp/place/name"/> : <xsl:value-of select="fp/forum/name"/> : <xsl:value-of select="fp/org/name"/> : <xsl:value-of select="fp/host/name"/></title>
      </head>
      <body>
	<a>
	  <xsl:attribute name="href">
	    <xsl:value-of select="/fp/host/url"/>
	    <xsl:text>/</xsl:text>
	    <xsl:value-of select="/fp/org/ref"/>
	    <xsl:text>/</xsl:text>
	    <xsl:value-of select="/fp/forum/ref"/>
	    <xsl:text>?info=</xsl:text>
	    <xsl:value-of select="/fp/place/ref"/>
	  </xsl:attribute>
  
	<table width="100%" border="0">
	  <xsl:for-each select="fp/slides/slide">

	    <!-- This version: one slide per row -->
	    <tr><td align="center"><xsl:apply-templates select="."/></td></tr>

	    <!-- This version: two slides per row -->
	    <!-- <xsl:variable name="node-position" select="position()"/>
	    <xsl:if test="$node-position mod 2 = 1">
	      <tr>
		<td align="center"><xsl:apply-templates select="."/></td>
		<td align="center"><xsl:apply-templates select="../slide[$node-position + 1]"/></td>
	      </tr>
	    </xsl:if>
	    -->
	  </xsl:for-each>
	</table>
	</a>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>
