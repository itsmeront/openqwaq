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
    
    &gt; <xsl:value-of select="fp/user/name"/>
  </xsl:template>
  <xsl:template name="page_type">
    <xsl:text>User</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    <xsl:value-of select="fp/user/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>user</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/user/name"/>
  </xsl:template>
  <xsl:template name="header_content">
  </xsl:template>
  <xsl:template name="rssLink">
    <a id="rss_link">
    <xsl:attribute name="href"><xsl:value-of select="fp/host/url"/>/rss2.xml?userID=<xsl:value-of select="fp/user/url"/>&amp;category=docs,presence,forums</xsl:attribute>
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
      <h3>User Info</h3>
      <img width="180" alt="User profile image" class="main_image">
      <xsl:attribute name="alt"><xsl:value-of select="fp/user/name"/></xsl:attribute>
      <xsl:attribute name="title"><xsl:value-of select="fp/user/name"/></xsl:attribute>
      <xsl:attribute name="src">/_small_picture.png?id=<xsl:value-of select="fp/user/ref"/></xsl:attribute>
      </img>
      <p><strong>
	  <xsl:choose>
	    <xsl:when test="fp/user/emailLabel"><xsl:value-of select="fp/user/emailLabel"/></xsl:when>
	    <xsl:otherwise>Email</xsl:otherwise>
	  </xsl:choose>: </strong><a>
        <xsl:attribute name="href">mailto:<xsl:value-of select="fp/user/email"/></xsl:attribute>
        <xsl:value-of select="fp/user/email"/></a></p>
      <xsl:if test="string-length(/fp/user/summary)>0">
        <p><strong>Profile: </strong><xsl:value-of select="/fp/user/summary"/></p>
      </xsl:if>
    </li>
    <li>
      <h3>Organizations in Common</h3>
      <ul>
	<xsl:choose>
	  <xsl:when test="fp/orgs/org">
        <xsl:for-each select="fp/orgs/org">
          <li><a>
            <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="url"/></xsl:attribute>
            <xsl:value-of select="name"/></a></li>
        </xsl:for-each>
	</xsl:when>
	<xsl:otherwise>
	  <p>&nbsp;None.</p>
	</xsl:otherwise>
	</xsl:choose>
      </ul>
    </li>
  </xsl:template>
</xsl:stylesheet>
