<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- Style Sheet for displaying Forums Status -->
  <!-- Greg Nuyens, Nov 16 2007 -->

<xsl:template match="/">
  <html>
	<head>
	<meta http-equiv="refresh" content="300"/>
	</head>
  <body>
    <h2>Forums Status (<xsl:value-of select="ok/timestamp"/>)
	(<a><xsl:attribute name="href">/forums/logs/</xsl:attribute>logs</a>)
    </h2>
      <table border="1">
	<!-- Qwaq Orange is #F48024 -->
	<!-- Qwaq Grey is #6A737B -->
	<tr bgcolor="#6A737B">
	  <!-- leaving this black 
	  <span style="color:#000000">
	    <th align="center">Overall</th>
	  </span> -->
	    <th align="center">Overall</th>
	    <th align="center"> </th>
	</tr>
	<tr>
	  <td>Server Version</td>
	  <td> <xsl:value-of select="ok/version"/></td>
	</tr>
	<tr>
	  <td>Host</td>
	  <td> <xsl:value-of select="ok/host"/></td>
	</tr>
	<tr>
	  <td>Total Users</td>
	  <td> <xsl:value-of select="ok/activeUsers"/></td>
	</tr>
	<tr>
	  <td>Query Stamp</td>
	  <td> <xsl:value-of select="ok/timestamp"/></td>
	</tr>
	<tr>
	  <td>Total Memory</td>
	  <td> <xsl:value-of select="ok/memory"/></td>
	</tr>
	<tr>
	  <td>Full GCs</td>
	  <td> <xsl:value-of select="ok/fullGCs"/></td>
	</tr>
	<tr>
	  <td>Tenures</td>
	  <td> <xsl:value-of select="ok/tenures"/></td>
	</tr>
	<tr>
	  <td>Build String</td>
	  <td> <xsl:value-of select="ok/build"/></td>
	</tr>
      </table>

      <!-- List of client connections -->
      <h4>Active Connections</h4>
      <table border="1">
	<tr bgcolor="#6A737B">
	    <th align="center">Client IP</th>
	    <th align="center">User</th>
	    <th align="center">Organization</th>
	    <th align="center">Duration</th>
	    <th align="center">Last Active</th>
	    <th align="center">Version</th>
	</tr>
        <xsl:for-each select="ok/spHost/ok/connections/client">
          <tr>
	    <td><xsl:value-of select="ip"/></td>
	    <td><xsl:value-of select="name"/></td>
	    <td><xsl:value-of select="org"/></td>
	    <td><xsl:value-of select="duration"/></td>
	    <td><xsl:value-of select="active"/></td>
	    <td><xsl:value-of select="version"/></td>
	  </tr>
	</xsl:for-each>
      </table>

      <!-- Output the Router Host information -->
    <xsl:for-each select="ok/routerHost">
      <h4>Router:
	<xsl:value-of select="@name"/>
	(<a><xsl:attribute name="href">http://<xsl:value-of select="@name"/>/router/logs/</xsl:attribute>logs</a>)
      </h4>
      <xsl:for-each select="failed">
	<span style="color:red">HOST IS DOWN: <xsl:value-of select="."/></span><p/>
      </xsl:for-each>
      <xsl:for-each select="ok">
        Active Forums: <xsl:value-of select="activeIslands"/>
        Active Clients: <xsl:value-of select="activeClients"/> 
        <table border="1" >
	  <tr bgcolor="#6A737B">
	    <th>Forum</th><th>Users</th>
	  </tr>
        <xsl:for-each select="forum">
	  <tr>
	    <td><xsl:value-of select="name"/></td>
	    <td><xsl:value-of select="users"/></td>
	  </tr>
        </xsl:for-each>
	</table>
	Routers:
        <table border="1" >
	  <tr bgcolor="#6A737B">
	    <th>Name</th><th>Users</th>
	  </tr>
        <xsl:for-each select="router">
	  <tr>
	    <td><xsl:value-of select="name"/></td>
	    <td><xsl:value-of select="users"/></td>
	  </tr>
        </xsl:for-each>
	</table>
	Connections:
        <table border="1" >
	  <tr bgcolor="#6A737B">
	    <th>Source</th><th>User</th><th>Duration</th><th>Version</th>
	  </tr>
        <xsl:for-each select="client">
	  <tr>
	    <td><xsl:value-of select="ip"/></td>
	    <td><xsl:value-of select="name"/></td>
	    <td><xsl:value-of select="duration"/></td>
	    <td><xsl:value-of select="version"/></td>
	  </tr>
        </xsl:for-each>
        </table>
      </xsl:for-each>
    </xsl:for-each>


      <!-- Output the Application Host information -->
    <xsl:for-each select="ok/appHost">
      <h4> Application Host: 
	<xsl:value-of select="@name"/>
	(<a><xsl:attribute name="href">http://<xsl:value-of select="@name"/>/applications/logs/</xsl:attribute>logs</a>)
      </h4>
      <xsl:for-each select="failed">
	<span style="color:red">HOST IS DOWN: <xsl:value-of select="."/></span><p/>
      </xsl:for-each>
      <xsl:for-each select="ok">
        Screens (active/total): <xsl:value-of select="activeScreens"/> /
        <xsl:value-of select="totalScreens"/> 
        <table border="1" >
	  <tr bgcolor="#6A737B">
	    <th>Screen</th><th>Application</th><th>Account</th><th>Description</th>
	  </tr>
	  <xsl:for-each select="screen">
	    <tr>
	      <td><xsl:value-of select="@number"/></td>
	      <td><xsl:value-of select="name"/></td>
	      <td><xsl:value-of select="account"/></td>
	      <td><xsl:value-of select="description"/></td>
	    </tr>
          </xsl:for-each>
        </table>
      </xsl:for-each>
    </xsl:for-each>


    <!-- Output the Video Server information -->
    <xsl:for-each select="ok/netvidHost">
      <h4> Video Server: 
	<xsl:value-of select="@name"/>
	(<a><xsl:attribute name="href">http://<xsl:value-of select="@name"/>/netvid/logs/</xsl:attribute>logs</a>)
      </h4>
      <xsl:for-each select="failed">
	<span style="color:red">HOST IS DOWN: <xsl:value-of select="."/></span><p/>
      </xsl:for-each>
      <xsl:for-each select="ok">
	Servers: <xsl:value-of select="serverCount"/>
	Assets: <xsl:value-of select="assetCount"/>
        <table border="1" >
	  <tr bgcolor="#6A737B">
	    <th align="center">State</th>
	    <th align="center">ID</th>
	    <th align="center">Asset</th>
	  </tr>
	  <xsl:for-each select="server">
	    <tr>
	      <td><xsl:value-of select="state"/></td>
	      <td><xsl:value-of select="@id"/></td>
	      <td><xsl:value-of select="asset"/></td>
	    </tr>
          </xsl:for-each>
        </table>
      </xsl:for-each>
    </xsl:for-each>


  </body>
  </html>
</xsl:template>

</xsl:stylesheet>
