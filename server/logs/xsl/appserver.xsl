<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
  <html>
  <body>
      <table border="1" >
	<tr bgcolor="#6A737B">
	  <th align="center" width="150px">Date</th>
	  <th align="center" width="80px">Type</th>
	  <th align="center" width="200px">Source</th>
	  <th align="center">Description</th>
	</tr>
    	<xsl:for-each select="log/*">
	  <tr>
	    <td><xsl:value-of select="Date"/></td>
	    <td><xsl:value-of select="name(.)"/></td>
	    <td><xsl:value-of select="Source"/>
	       <xsl:if test="name(.)='Activity'">
		 <xsl:value-of select="User"/>
	       </xsl:if>
	    </td>
	    <td><xsl:value-of select="Description"/>
	       <xsl:if test="name(.)='Activity'">
		 <xsl:value-of select="Action"/>:
		 <xsl:value-of select="Extra"/>
	       </xsl:if>
	    </td>
	  </tr>
	  <xsl:if test="name(.)='Error'">
	    <tr><td colspan="4">
	      <pre><xsl:value-of select="Details"/></pre>
	      <pre><xsl:value-of select="Stack"/></pre>
	    </td></tr>
	  </xsl:if>
      	</xsl:for-each>
      </table>
  </body>
  </html>
</xsl:template>

</xsl:stylesheet>
