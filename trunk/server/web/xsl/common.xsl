<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE xsl:stylesheet [
	  <!ENTITY nbsp "&#160;">
	  <!ENTITY copy "&#169;">
]>
<xsl:stylesheet version="2.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		>

  <!-- On the Qwaq Enterprise Server, the various files are at different locations - there is no one relative path
       that can reach /xsl/mumble from all of them. Note that these references are in the generated html, so they
       are interpreted relative to the HTTP URL.
       However, when testing in a different environment such as the file system, /xsl/mumble is not defined.
       The secureBase variable can be defined to provide the location of /xsl/mumble relative to the test source. -->
  <!-- Use this version for running in the Qwaq Enterprise Server -->
  <xsl:variable name="secureBase">/xsl/</xsl:variable>
  <!-- Use this version for testing hand-written .xml files in the file system -->
<!--   <xsl:variable name="secureBase"></xsl:variable> -->

<xsl:output method="html" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd" doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN" />

  <xsl:variable name="supportUrl">
    <xsl:choose>
      <xsl:when test="/fp/host/support">
	<xsl:value-of select="/fp/host/support"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="/fp/host/url"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:template name="support_contact">
    <xsl:choose>
      <xsl:when test="/fp/contact">
	<xsl:value-of select="/fp/contact/ref"/>
      </xsl:when>
      <xsl:otherwise>support@openqwaq.com</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="host">
    <a>
    <xsl:attribute name="href"><xsl:value-of select="url"/></xsl:attribute>
    <xsl:value-of select="name"/></a>
  </xsl:template>
  <xsl:template match="org">
    <a>
    <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="ref"/></xsl:attribute>
    <xsl:value-of select="name"/></a>
  </xsl:template>
  <xsl:template match="forum">
    <a>
    <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="ref"/></xsl:attribute>
    <xsl:value-of select="name"/></a>
  </xsl:template>
  <xsl:template match="place">
    <a>
    <xsl:attribute name="href"><xsl:value-of select="/fp/host/url"/>/<xsl:value-of select="/fp/org/ref"/>/<xsl:value-of select="/fp/forum/ref"/>?info=<xsl:value-of select="ref"/></xsl:attribute>
    <xsl:value-of select="name"/></a>
  </xsl:template>
  <xsl:template name="upload">
    <xsl:param name="placeTarget"><xsl:value-of select="fp/place/ref"/></xsl:param>
    <li>
      <h3>Upload a File</h3>
      <form action="/upload" enctype="multipart/form-data" method="POST" id="form-upload">
        <input type="hidden" name="organization">
        <xsl:attribute name="value"><xsl:value-of select="fp/org/ref"/></xsl:attribute>
        </input>
        <input type="hidden" name="forum">
        <xsl:attribute name="value"><xsl:value-of select="fp/forum/ref"/></xsl:attribute>
        </input>
        <input type="hidden" name="place">
        <xsl:attribute name="value"><xsl:value-of select="$placeTarget"/></xsl:attribute>
        </input>
        <input name="contents" type="file" />
        <input type="submit" value="Upload file" />
      </form>
    </li>
  </xsl:template>
  <xsl:template name="recent_activity_table">
        <xsl:call-template name="rssLink"/>
        
        <table cellspacing="0" id="recent_activity">
          <caption>
          Recent Activity
          </caption>
          <xsl:call-template name="header_row"/>
          
          <tbody><xsl:call-template name="content_rows"/></tbody>
          
        </table>
  </xsl:template>
  <xsl:template name="recent_activity_header_row">
    <xsl:param name="tableType"><xsl:call-template name="page_type"/></xsl:param>
<thead> 
    <tr>
      <th class="item">Item</th>
      <th class="action">Action</th>
      <xsl:if test="$tableType!='User'">
        <th class="user">User</th>
       </xsl:if>
      <th class="date">Date <span>(UTC)</span></th>
    </tr>
 </thead> 
 </xsl:template>
  <xsl:template name="recent_activity_content_rows">
    <xsl:param name="tableType"><xsl:call-template name="page_type"/></xsl:param>
    <xsl:for-each select="fp/rss/channel/item">
      <tr>
        <td class="item"><a class="thumb">
          <xsl:attribute name="href"><xsl:value-of select="link"/></xsl:attribute>
          <img width="53">
          <xsl:attribute name="src">
	    <xsl:choose>
	      <xsl:when test="thumbnail">
		<xsl:value-of select="thumbnail"/>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:choose>
		  <xsl:when test="category='forums'"><xsl:copy-of select="$secureBase"/>images/icon-forum.gif</xsl:when>
		  <xsl:when test="category='forums'"><xsl:copy-of select="$secureBase"/>images/icon-forum.gif</xsl:when>
		  <xsl:when test="category='docs'"><xsl:copy-of select="$secureBase"/>images/icon-file.gif</xsl:when>
		  <xsl:when test="category='presence'"><xsl:copy-of select="$secureBase"/>images/icon-user.gif</xsl:when>
		  <xsl:otherwise><xsl:copy-of select="$secureBase"/>images/logo-company.gif</xsl:otherwise>
		</xsl:choose>
	      </xsl:otherwise>
	    </xsl:choose>
	  </xsl:attribute>
          </img></a><a>
          <xsl:attribute name="href"><xsl:value-of select="link"/></xsl:attribute>
          <xsl:value-of select="title"/></a>
            <br />
            <span class="location">
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
            </span>
          </td>
        <td class="action">
	  <xsl:choose>
	    <xsl:when test="category='forums'">Forum </xsl:when>
	    <xsl:when test="category='forums'">Forum </xsl:when>
	    <xsl:when test="category='docs'">Document </xsl:when>
	    <xsl:when test="category='presence'">User </xsl:when>
	    <xsl:otherwise><xsl:value-of select="category"/><xsl:text> </xsl:text></xsl:otherwise>
	  </xsl:choose>
	  <xsl:value-of select="description"/>
	</td>
        <xsl:if test="$tableType!='User'">
          <td class="user">
	    <xsl:choose>
	      <xsl:when test="user">
		<xsl:value-of select="user"/>
	      </xsl:when>
	      <xsl:otherwise>
		(auto)
	      </xsl:otherwise>
	    </xsl:choose>
	  </td>
        </xsl:if>
        <td class="date"><xsl:value-of select="pubDate"/></td>
      </tr>
    </xsl:for-each>
  </xsl:template>
  <!-- General template -->
  <xsl:template match="/">
    <html>
    <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
 <meta http-equiv="X-UA-Compatible" content="IE=8" />
   <title>
    <xsl:call-template name="page_title"/>
    </title>
    <link rel="stylesheet" href="{$secureBase}css/reset.css" type="text/css" media="screen, projection"/>
    <link rel="stylesheet" href="{$secureBase}css/screen.css" type="text/css" media="screen, projection"/>
    <link rel="stylesheet" href="{$secureBase}css/print.css" type="text/css" media="print"/>
    <script type="text/javascript" src="{$secureBase}js/jquery-1.3.2.min.js"></script>
    <script type="text/javascript" src="{$secureBase}js/jquery.tablesorter.min.js"></script>
    <script type="text/javascript" src="{$secureBase}js/jquery.listnav.min-2.1.js"></script>
    <script type="text/javascript" src="{$secureBase}js/scripts.js"></script>
    <script type="text/javascript" src="{$secureBase}js/jquery.matchingheights.js"></script>
    <!--[if IE]>
    <link rel="stylesheet" href="{$secureBase}css/ie.css" type="text/css" media="screen" />
    <script type="text/javascript" src="{$secureBase}js/ie.js"></script>
<![endif]-->
    <!--[if IE 8]>
    <link rel="stylesheet" href="{$secureBase}css/ie8.css" type="text/css" media="screen" />
    <script type="text/javascript" src="{$secureBase}js/ie8.js"></script>
<![endif]-->
    <!--[if IE 7]>
    <link rel="stylesheet" href="{$secureBase}css/ie7.css" type="text/css" media="screen" />
    <script type="text/javascript" src="{$secureBase}js/ie7.js"></script>
<![endif]-->
    <!--[if IE 6]>
    <link rel="stylesheet" href="{$secureBase}css/ie6.css" type="text/css" media="screen" />
    <script type="text/javascript" src="{$secureBase}js/ie6.js"></script>
<![endif]-->
    </head>
    <body>
    <xsl:attribute name="class">
    <xsl:call-template name="body_class"/>
    </xsl:attribute>
    <div id="header">
      <h1><a href="http://www.openqwaq.com/">OpenQwaq</a></h1>
      <img src="{$secureBase}images/logo-openqwaq.png" width="51" height="63" alt="OpenQwaq" id="logo-qwaq" />
      <ul id="header_links">
        <xsl:if test="fp/requester/ref">
          <li><a>
            <xsl:attribute name="href">/user?id=<xsl:value-of select="fp/requester/ref"/></xsl:attribute>
            User Profile</a></li>
        </xsl:if>
        <li>
	  <a style="color: white; visited: white" href="{$supportUrl}/docs/userguide/wwhelp/wwhimpl/common/html/wwhelp.htm">User Guide</a>
	</li>
        <xsl:if test="fp/requester/ref">
          <li><a>
	    <xsl:attribute name="href">
	      <xsl:text>mailto:</xsl:text>
	      <xsl:call-template name="support_contact"/>
	      <xsl:text>?subject=</xsl:text>
	      <xsl:value-of select="fp/url"/>
	    </xsl:attribute>
            Support</a></li>
          <li><a href="/logout" title="Logout">Logout</a></li>
        </xsl:if>
      </ul>
    </div>
    <div id="content">
      <div id="content_header"> <img src="{$secureBase}images/logo-company.gif" width="110" height="74" alt="Company logo" id="logo-company" />
        <p id="breadcrumb_nav">
          <xsl:call-template name="breadcrumb_nav"/>
        </p>
        <h2>
          <xsl:call-template name="page_type"/>:
          <xsl:call-template name="item_displayed"/>
        </h2>
          <xsl:call-template name="header_content"/>
      </div>
      <div id="content_main">
          <xsl:call-template name="left_column_content"/>
      </div>
      <div id="content_sub">
        <ul>
          <xsl:call-template name="right_column_content"/>
          
        </ul>
      </div>
    </div>
    </body>
    </html>
  </xsl:template>
</xsl:stylesheet>
