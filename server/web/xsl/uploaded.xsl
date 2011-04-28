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
    
    &gt; <xsl:apply-templates select="fp/place"/>
  </xsl:template>
  <xsl:template name="page_type">
    <xsl:text>Upload Successful</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    <xsl:value-of select="fp/place/name"/> : <xsl:value-of select="fp/forum/name"/> : <xsl:value-of select="fp/org/name"/> : <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>upload</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/file/name"/>
  </xsl:template>
  <xsl:template name="rssLink">
  </xsl:template>
  <xsl:template name="header_row">
  </xsl:template>
  <xsl:template name="content_rows">
  </xsl:template>
  <xsl:template name="header_content">
  	<p style="margin-top:16px;">The file <xsl:value-of select="fp/file/name"/> will appear at Dropbox when someone is in the forum, and in the 
	<a>
	  <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="/fp/forum/ref"/></xsl:attribute>
	  recent document list
	</a>
	when the forum is saved.</p>
  </xsl:template>
  <xsl:template name="left_column_content">
  </xsl:template>
  <xsl:template name="right_column_content">
  </xsl:template>
</xsl:stylesheet>
