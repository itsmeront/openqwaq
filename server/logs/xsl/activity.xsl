<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
  <html>
  <body>
      <table border="1" >
	<tr bgcolor="#6A737B">
	  <th align="center" width="150px">Date</th>
	  <th align="center">Host</th>
	  <th align="center">Action</th>
	  <th align="center">User</th>
	  <th align="center">Organization</th>
	  <th align="center">Extra</th>
	</tr>
    	<xsl:for-each select="log/*">
	  <tr>
	    <td><xsl:value-of select="Date"/></td>
	    <td><xsl:value-of select="Host"/></td>
	    <td><xsl:value-of select="Action"/></td>
	    <td><xsl:value-of select="User"/></td>
	    <td><xsl:value-of select="Organization"/></td>
	    <td><xsl:value-of select="Extra"/></td>
	  </tr>
      	</xsl:for-each>
      </table>
  </body>
  </html>
</xsl:template>

</xsl:stylesheet>
