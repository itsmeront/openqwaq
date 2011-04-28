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
    
    &gt; <xsl:value-of select="fp/org/name"/>
  </xsl:template>
  <xsl:template name="page_type">
    <xsl:text>Organization</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    <xsl:value-of select="fp/org/name"/> : <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>org</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/org/name"/>
  </xsl:template>
   <xsl:template name="header_content">
  </xsl:template>
 <xsl:template name="rssLink">
    <a id="rss_link">
    <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/rss2.xml?organization=<xsl:value-of select="fp/org/ref"/>&amp;category=docs,presence,forums</xsl:attribute>
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
      <h3>Organization Lobby</h3>
      <p><a class="lobby_link">
        <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/<xsl:value-of select="fp/org/ref"/>?go=Lobby</xsl:attribute>
        Enter organization lobby</a></p>
    </li>
    <li>
      <h3>Forums</h3>
      <ul id="forums_list">
        <xsl:for-each select="fp/forums/forum">
          <li><a class="thumb">
            <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="ref"/></xsl:attribute>
            <img border="0" width="53" alt="Forum">
            <xsl:attribute name="title"><xsl:value-of select="name"/></xsl:attribute>
            <xsl:attribute name="src">/preview.jpg?organization=<xsl:value-of select="/fp/org/ref"/>&amp;forum=<xsl:value-of select="ref"/></xsl:attribute>
            </img></a><a>
            <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="ref"/></xsl:attribute>
            <xsl:value-of select="name"/></a></li>
        </xsl:for-each>
      </ul>
    </li>
  </xsl:template>
</xsl:stylesheet>
