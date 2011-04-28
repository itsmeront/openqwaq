<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE xsl:stylesheet [
	  <!ENTITY nbsp "&#160;">
	  <!ENTITY copy "&#169;">
]>
<xsl:stylesheet version="2.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		>
  <xsl:variable name="buttonHeight">40</xsl:variable>
  <xsl:variable name="activityThumbnailHeight">75</xsl:variable>
  <xsl:variable name="thumbnailHeight">175</xsl:variable>
  <xsl:variable name="previewHeight">300</xsl:variable>
  <xsl:variable name="breadcrumbWidth">175</xsl:variable>
  <xsl:variable name="secureBase">/</xsl:variable>

  <xsl:variable name="supportHost">
    <xsl:choose>
      <xsl:when test="/fp/host/support">
	<xsl:value-of select="/fp/host/support"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="/fp/host/ref"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <!-- <orgItem><ref>Foo_Bar</ref><name>Foo Bar</name></orgItem>
       to
       <td><a href="http://host/Foo_Bar">Foo Bar</a></td>
       -->
  <xsl:template match="host">
    <a>
      <xsl:attribute name="href"><xsl:value-of select="url"/></xsl:attribute>
      <xsl:value-of select="name"/>
    </a>
  </xsl:template>

  <xsl:template match="org">
    <a>
      <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="ref"/></xsl:attribute>
      <xsl:value-of select="name"/>
    </a>
  </xsl:template>

  <xsl:template match="forum">
    <a>
      <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="ref"/></xsl:attribute>
      <xsl:value-of select="name"/>
    </a>
  </xsl:template>

  <xsl:template match="forum" mode="thumbnail">
    <img border="0" height="{$thumbnailHeight}" alt="Forum">
      <xsl:attribute name="title"><xsl:value-of select="name"/></xsl:attribute>
      <xsl:attribute name="src"><xsl:value-of select="/fp/host/url"/>/preview.jpg?organization=<xsl:value-of select="/fp/org/ref"/>&amp;forum=<xsl:value-of select="ref"/></xsl:attribute>
    </img><br/>
    <a>
      <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="ref"/></xsl:attribute>
      <xsl:value-of select="name"/>
    </a>
  </xsl:template>

  <xsl:template match="place">
    <a>
      <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="/fp/forum/ref"/>?info=<xsl:value-of select="ref"/></xsl:attribute>
      <xsl:value-of select="name"/>
    </a>
  </xsl:template>


  <xsl:template name="breadcrumbs">
    <tr>
      <td align="right" width="{$breadcrumbWidth}">
	<table border="0" cellspacing="0" cellpadding="0">
	  <xsl:call-template name="breadcrumbHeaders"/>
	</table>
      </td>
      <td>
	<table border="0" cellspacing="0" cellpadding="0">
	  <xsl:call-template name="breadcrumbLinks"/>
	</table>
      </td>
      <td align="right">
	<table border="0" cellspacing="0" cellpading="0">
	  <xsl:call-template name="preview"/>
	</table>
      </td>
    </tr>
  </xsl:template>

  <xsl:template name="rssListing">
    <!-- [rss logo] Recent activity: times are UTC
	 2009-07-21 13:14:45          [picture]          title-as-link/description
	 ...
      -->          
    <tr><td colspan="3"><br/><br/></td></tr>
    <tr><th align="right">
	<a STYLE="color: white; visited: white">
	  <xsl:call-template name="rssLink"/>
	  <img border="0" align="top" alt="RSS feed" title="RSS feed" src="{$secureBase}images/feed.png"/>
	</a>
	Recent activity:</th><td colspan="2" valign="top">&nbsp;times are UTC</td></tr>
    <tr>
      <td colspan="3">
	<table>
	  <xsl:for-each select="fp/rss/channel/item">
	    <tr>
	      <td>
		<xsl:value-of select="pubDate"/>
	      </td>
	      <td>
		<img height="{$activityThumbnailHeight}">
		  <xsl:attribute name="src">
		    <xsl:choose>
		      <xsl:when test="thumbnail">
			<xsl:value-of select="thumbnail"/>
		      </xsl:when>
		      <xsl:otherwise>
			<xsl:text>/images/logo.gif</xsl:text>
		      </xsl:otherwise>
		    </xsl:choose>
		  </xsl:attribute>
		  <xsl:attribute name="style">
		    border: 3px
		    <xsl:choose>
		      <xsl:when test="category='presence'">
			#F37033
		      </xsl:when>
		      <xsl:when test="category='docs'">
			#455560
		      </xsl:when>
		      <xsl:otherwise>
			white
		      </xsl:otherwise>
		    </xsl:choose>
		    solid
		  </xsl:attribute>
		</img>
	      </td>
	      <td>
		<a><xsl:attribute name="href"><xsl:value-of select="link"/></xsl:attribute><xsl:value-of select="title"/></a><br/>
		<xsl:value-of select="description"/>
		 <xsl:if test="user">
		  <xsl:text> by </xsl:text>
		  <xsl:value-of select="user"/>
		 </xsl:if>
		 <xsl:if test="org">
                  <xsl:text> in </xsl:text>
                  <xsl:value-of select="org"/>
                </xsl:if>
		<xsl:if test="forum">
		  <xsl:choose>
		    <xsl:when test="org">
		      <xsl:text>/</xsl:text>
		    </xsl:when>
		    <xsl:otherwise>
		      <xsl:text> in </xsl:text>
		    </xsl:otherwise>
		  </xsl:choose>
                  <xsl:value-of select="forum"/>
		</xsl:if>
		<xsl:text>.</xsl:text>
	      </td>
	    </tr>
	  </xsl:for-each>
	</table>
      </td>
    </tr>
  </xsl:template>

  <xsl:template name="picture">
    <xsl:param name="link"/>
    <xsl:param name="src"/>
    <xsl:param name="alt"/>
    <xsl:param name="title"/>
    <xsl:param name="caption"/>
    <xsl:param name="captionExtension">
      <p>
	
      </p>
    </xsl:param>

    <table border="0" cellpadding="0" cellspacing="0">
      <tr>
	<td>
	  <a> 
	    <xsl:attribute name="href"><xsl:value-of select="$link"/></xsl:attribute>
	    <img border="0" height="{$previewHeight}" align="right">
	      <xsl:attribute name="alt"><xsl:value-of select="$alt"/></xsl:attribute>
	      <xsl:attribute name="title"><xsl:value-of select="$title"/></xsl:attribute>
	      <xsl:attribute name="src"><xsl:value-of select="$src"/></xsl:attribute>
	    </img>
	  </a>
	</td>
      </tr>
      <tr>
	<td align="center">
	  <em><xsl:value-of select="$caption"/></em>
	</td>
      </tr>
    </table>
  </xsl:template>

  <xsl:template match="/">

    <html>

      <head>
	<title><xsl:call-template name="pageTitle"/></title>
      </head>

      <body>
	<div style="font-family: sans-serif">
	  <!-- Banner Header
	       [logo] Host Page                           [various help links]
	    -->
	  <table width="100%" border="0" cellspacing="0" bgcolor="#455560">
	    <tr>
	      <td width="68">
		<a style="color: #455560; visited: #455560" href="http://www.qwaq.com">
		  <img border="0" alt="Qwaq" src="{$secureBase}images/logo.gif"/>
		</a>
	      </td>
	      <td><span style="color: white"><xsl:call-template name="pageType"/> Page</span></td>
	      <td align="right">
		<small>
		  <xsl:if test="fp/requester/ref">
		    <a style="color: white; visited: white" href="/logout" title="Log out">You are:</a>
		    <xsl:text> </xsl:text>
		    <a style="color: white; visited: white" title="Your page">
		      <xsl:attribute name="href">/user?id=<xsl:value-of select="fp/requester/ref"/></xsl:attribute>
		      <xsl:value-of select="fp/requester/name"/>
		    </a>
		    &nbsp;
		  </xsl:if>
		  <a style="color: white; visited: white" href="{$supportUrl}/support/visitor.php">First time user? Click here</a> 
		  &nbsp;
		  <a style="color: white; visited: white" href="{$supportUrl}/docs/userguide/wwhelp/wwhimpl/common/html/wwhelp.htm">User Guide</a> 
		  &nbsp;
		  <a style="color: white; visited: white">
		    <xsl:attribute name="href">
		      <xsl:text>mailto:</xsl:text>
		      <xsl:value-of select="/fp/contact/ref"/>
		      <xsl:text>?subject=</xsl:text>
		      <xsl:value-of select="fp/url"/></xsl:attribute>Support</a>
		  &nbsp;
		</small>
	      </td>
	    </tr>
	  </table>

	  <p></p>
	  <table width="100%" border="0" cellspacing="0" cellpadding="0">
	    <!-- {breadcrumbs, e.g.}
		 Organization: QTest
		 Forum: Test_mainforum1
		 Host: forums.qwaq.com
	      -->
	    <xsl:call-template name="breadcrumbs"/>

	    <!-- {mainAction, e.g.}
		 Upload a file: [arrow]
	      -->
	    <tr><td colspan="3"><br/></td></tr>
	    <xsl:call-template name="mainAction"/>

	    <xsl:call-template name="recentActivity"/>
	  </table>

	  <!-- Footer -->
	  <p></p>
	  <p></p>
	  <hr/>
	  <table>
	    <tr>
	      <td><small>Qwaq Forums are virtual environments used to facilitate interactive online meetings, application access, workflow, project and program management processes, real-time document editing, and online training.</small></td>
	      <td align="right" width="33%"><small>&copy; 2009 <a href="http://www.qwaq.com">Qwaq, Inc</a>.</small></td>
	    </tr>
	  </table>
	</div>
      </body>
    </html>

  </xsl:template>
</xsl:stylesheet>
