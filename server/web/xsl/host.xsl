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
    <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="page_type">
    <xsl:text>Host</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>host</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="header_content">
  </xsl:template>
  <xsl:template name="rssLink">
    <a id="rss_link">
    <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/rss2.xml?category=docs,presence,forums</xsl:attribute>
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
      <h3>Organizations</h3>
      <ul id="orgs_list">
	<xsl:choose>
	  <xsl:when test="fp/orgs/org">
        <xsl:for-each select="fp/orgs/org">
          <li><a>
            <xsl:attribute name="href"><xsl:value-of select="/fp/url"/><xsl:value-of select="ref"/></xsl:attribute>
            <xsl:value-of select="name"/></a></li>
        </xsl:for-each>
	</xsl:when>
	<xsl:otherwise>
	  <p>You have no organizations that have been enabled for Forum Pages.</p>
	  <p>Please contact your organizations' contacts, or 
	  <a>
	    <xsl:attribute name="href">
	      <xsl:text>mailto:</xsl:text>
	      <xsl:call-template name="support_contact"/>
	      <xsl:text>?subject=Enable%20Forum%20Pages</xsl:text>
	    </xsl:attribute>
	  <xsl:call-template name="support_contact"/>
	  </a>.</p>
	</xsl:otherwise>
	</xsl:choose>
      </ul>
    </li>
  </xsl:template>
</xsl:stylesheet>
