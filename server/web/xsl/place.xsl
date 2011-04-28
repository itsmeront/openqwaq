<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE xsl:stylesheet [
	  <!ENTITY nbsp "&#160;">
	  <!ENTITY copy "&#169;">
]>
<xsl:stylesheet version="2.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		>
  <xsl:import href="common.xsl"/>
  <xsl:template name="breadcrumb_nav">
    <xsl:apply-templates select="fp/host"/>
    
    &gt;
    <xsl:apply-templates select="fp/org"/>
    
    &gt;
    <xsl:apply-templates select="fp/forum"/>
    
    &gt; <xsl:value-of select="fp/place/name"/>
  </xsl:template>
  <xsl:template name="page_type">
    <xsl:text>Landmark</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    <xsl:value-of select="fp/place/name"/> : <xsl:value-of select="fp/forum/name"/> : <xsl:value-of select="fp/org/name"/> : <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>place</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/place/name"/>
  </xsl:template>
  <xsl:template name="header_content">
  </xsl:template>
  <xsl:template name="rssLink">
  </xsl:template>
  <xsl:template name="header_row">
  </xsl:template>
  <xsl:template name="content_rows">
    <tr>
      <td>Recent activity information is not available for landmarks.</td>
    </tr>
  </xsl:template>
  <xsl:template name="left_column_content">
     <xsl:call-template name="recent_activity_table"/>
 </xsl:template>
  <xsl:template name="right_column_content">
    <li>
      <h3>Enter</h3>
      <p><a class="lobby_link">
      <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/<xsl:value-of select="fp/org/ref"/>/<xsl:value-of select="fp/forum/ref"/>?go=<xsl:value-of select="fp/place/ref"/></xsl:attribute>
        Enter Forum at landmark</a></p>
    </li>
    <xsl:call-template name="upload"/>
  </xsl:template>
</xsl:stylesheet>
