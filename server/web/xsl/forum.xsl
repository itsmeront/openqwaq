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
    
    &gt; <xsl:value-of select="fp/forum/name"/>
  </xsl:template>
  <xsl:template name="page_type">
    <xsl:text>Forum</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    <xsl:value-of select="fp/forum/name"/> : <xsl:value-of select="fp/org/name"/> : <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>forum</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/forum/name"/>
  </xsl:template>
  <xsl:template name="header_content">
  </xsl:template>
  <xsl:template name="rssLink">
    <a id="rss_link">
    <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/rss2.xml?organization=<xsl:value-of select="fp/org/ref"/>&amp;forum=<xsl:value-of select="fp/forum/ref"/>&amp;category=docs,presence,forums</xsl:attribute>
    RSS Link</a>
  </xsl:template>
  <xsl:template name="header_row">
    <xsl:call-template name="recent_activity_header_row"/>
  </xsl:template>
  <xsl:template name="content_rows">
    <xsl:call-template name="recent_activity_content_rows"/>
  </xsl:template>
  <xsl:template name="left_column_content">
     <xsl:call-template name="recent_activity_table"/>
 </xsl:template>
 <xsl:template name="right_column_content">
    <li>
      <h3>Recent View</h3>
      <p class="caption">Click image to enter</p>
      <a>
      <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/<xsl:value-of select="fp/org/ref"/>/<xsl:value-of select="fp/forum/ref"/>?go=Home</xsl:attribute>
      <img class="main_image" width="260" alt="Forum" title="Recent picture from inside the Forum." id="reloader">
      <xsl:attribute name="src">/preview.jpg?organization=<xsl:value-of select="fp/org/ref"/>&amp;forum=<xsl:value-of select="fp/forum/ref"/></xsl:attribute>
      <xsl:attribute name="onload">setTimeout('document.getElementById(\'reloader\').src=\'/preview.jpg?organization=<xsl:value-of select="fp/org/ref"/>&amp;forum=<xsl:value-of select="fp/forum/ref"/>&amp;uncache=\'+new Date().getMilliseconds()', 10000)</xsl:attribute>
      </img></a>
      <p class="caption">To copy this <a target="_blank" href="{$supportUrl}/support/tll-info.php">Landmark Link</a> for embedding link in email, IM, or calendar, just right click picture.</p>
      <xsl:if test="string-length(/fp/forum/summary)>0">
        <p><strong>Description: </strong><xsl:value-of select="/fp/forum/summary"/></p>
      </xsl:if>
    </li>
    <xsl:call-template name="upload">
      <xsl:with-param name="placeTarget" select="'Dropbox'"/>
    </xsl:call-template>
  </xsl:template>
</xsl:stylesheet>
