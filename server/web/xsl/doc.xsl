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
    <xsl:text>Document</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    <xsl:value-of select="fp/place/name"/> : <xsl:value-of select="fp/forum/name"/> : <xsl:value-of select="fp/org/name"/> : <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>file</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/place/name"/>
  </xsl:template>
   <xsl:template name="header_content">
  </xsl:template>
 <xsl:template name="rssLink">
    <a id="rss_link">
    <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/rss2.xml?organization=<xsl:value-of select="fp/org/ref"/>&amp;forum=<xsl:value-of select="fp/forum/ref"/>&amp;doc=<xsl:value-of select="fp/place/ref"/>&amp;category=docs</xsl:attribute>
    RSS Feed</a>
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
      <p class="caption">Click image to enter Forum at document</p>
      <a>
      <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/<xsl:value-of select="fp/org/ref"/>/<xsl:value-of select="fp/forum/ref"/>?go=<xsl:value-of select="fp/place/ref"/></xsl:attribute>
      <img class="main_image" width="260" alt="Document" title="Last preview of saved document">
      <xsl:attribute name="src">
	<xsl:choose>
	  <xsl:when test="/fp/rss/channel/item/thumbnail">
	    <xsl:value-of select="/fp/rss/channel/item/thumbnail"/>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:copy-of select="$secureBase"/><xsl:text>images/icon-file.gif</xsl:text>
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:attribute>
      </img></a>
      <p class="caption">To copy this <a target="_blank" href="{$supportUrl}/support/tll-info.php">Landmark Link</a> for embedding in email, IM, or calendar, just right click picture.</p>
    </li>
    <li>
      <h3>File <xsl:value-of select="fp/place/name"/></h3>

      <p>
	<xsl:choose>
	 <xsl:when test="fp/source">
		<a><xsl:attribute name="href"><xsl:value-of select="fp/source/url"/></xsl:attribute>
		Download</a> document from its repository.
	 </xsl:when>
	 <xsl:otherwise>
	 <a><xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/<xsl:value-of select="fp/org/ref"/>/<xsl:value-of select="fp/forum/ref"/>?view=<xsl:value-of select="fp/place/ref"/></xsl:attribute>
      View</a> directly, or 
	 <a><xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/<xsl:value-of select="fp/org/ref"/>/<xsl:value-of select="fp/forum/ref"/>?download=<xsl:value-of select="fp/place/ref"/></xsl:attribute>
      Download</a> to disk
	 </xsl:otherwise>
	</xsl:choose>
    </p>

    </li>
  </xsl:template>
</xsl:stylesheet>
