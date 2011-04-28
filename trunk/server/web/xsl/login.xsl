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
    Login
  </xsl:template>
  <xsl:template name="page_type">
    <xsl:text>Login</xsl:text>
  </xsl:template>
  <xsl:template name="page_title">
    Login : <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="body_class">
    <xsl:text>login</xsl:text>
  </xsl:template>
  <xsl:template name="item_displayed">
    <xsl:value-of select="fp/host/name"/>
  </xsl:template>
  <xsl:template name="header_row"/>  <!-- Make early-binding MSIE happy. -->
  <xsl:template name="content_rows"/>
  <xsl:template name="rssLink"/>
  <xsl:template name="header_content">
   <div id="login">
    <div id="form" class="login_section">
      <form action="/login" enctype="multipart/form-data" method="POST">
	  <input type="hidden" name="from"><xsl:attribute name="value"><xsl:value-of select="fp/from"/></xsl:attribute></input>
        <h3>Credentials</h3>
        <p><xsl:value-of select="fp/msg"/></p>
        <p>
          <label>Login:</label>
          <input type="text" id="user" name="user"><xsl:attribute name="value"><xsl:value-of select="fp/username"/></xsl:attribute></input>
        </p>
        <p>
          <label>Password:</label>
          <input type="password" id="password" name="password" />
        </p>
        <p>
          <input type="submit" value="Send"/>
        </p>
      </form>
    </div>
    <div class="login_section" id="column_one">
      <h3>What are OpenQwaq Forums? </h3>
      <p><a target="_blank" href="http://www.openqwaq.com">OpenQwaq</a> is an Application Collaboration solution, for meeting with colleagues to work or train using multiple shared applications, voice, webcam, text chat and other tools, in an immersive 3-D environment. </p>
      <p>This login is for <a href="{$supportUrl}/support/tp-info.php" target="_blank">Forum Pages</a>, which provide Web access to OpenQwaq, including links to launch the OpenQwaq client application.</p>
    </div>
    <div class="login_section" id="column_two">
      <h3>What to do Now </h3>
      <p>If you have never used OpenQwaq Forums before, you will need two things to use this web link: </p>
      <p>An invitation from an administrator of the Organization. </p>
      <p>The OpenQwaq software -- please visit <a href="{$supportUrl}/client"><xsl:copy-of select="$supportUrl"/>/client</a></p>
      <p>To obtain an invitation, please contact the source from which you received the web link, which may have been via email, calendar invitation or a web page. </p>
    </div>
    <div class="login_section" id="column_three">
      <h3>Having Trouble? </h3>
      <p>If you believe you may already have a login, you can use the password reset function available after installing the client software, on the client login screen. </p>
      <p>Please contact <a><xsl:attribute name="href">mailto:<xsl:call-template name="support_contact"/></xsl:attribute><xsl:call-template name="support_contact"/></a> if you have any questions. </p>
      <p>Thanks for using OpenQwaq. </p>
    </div>
    
  </div>
 </xsl:template>
  <xsl:template name="left_column_content">
 </xsl:template>
  <xsl:template name="right_column_content">
 </xsl:template>
 
</xsl:stylesheet>
