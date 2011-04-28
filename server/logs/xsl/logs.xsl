<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- Style Sheet for logs display -->

<xsl:template match="/">
  <html>
  <body>
      <table border="1" >
	<tr bgcolor="#6A737B">
	  <xsl:for-each select="logs/header/label">
	    <th align="center"><xsl:value-of select="."/></th>
	  </xsl:for-each>
	</tr>
    	<xsl:for-each select="logs/row">
	  <tr><xsl:for-each select="file">
	    <td><a><xsl:attribute name="href">
	      <xsl:value-of select="."/></xsl:attribute>
	      <xsl:value-of select="."/></a></td>
	  </xsl:for-each></tr>
      	</xsl:for-each>
      </table>
  </body>
  </html>
</xsl:template>

</xsl:stylesheet>
