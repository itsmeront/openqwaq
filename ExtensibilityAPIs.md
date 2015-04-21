# Extensibility and API's #

## Introduction ##

Now that OpenQwaq is open source, contributors can enhance the source directly.  However,  you may find that the OpenQwaq Python API in particular is a useful alternative for some purposes.   The sample applications below may also be illustrative.

## Available Approaches ##

OpenQwaq offers several areas for customization, enhancement and integration with external systems.

  * **Python API**:
    * client-side Python language API
    * runs on and has local access to one user's computer, and the network seen from that computer
    * allows rendering results in world, either 2-D on a panel, or by manipulating 3-D objects
    * examples:  multi-user whiteboard,  interactive 3-D data center interface, photo gallery,  stock graph,  avatar tracker
  * **Smart Rooms web interface**:
    * server-side web interface
    * for "offline" access, not designed for controlling what happens in a forum live
    * provides RSS access to activity feeds, and web access to document upload and download
    * typically accessed by programs running on another existing server
    * examples:  periodic query for user login activity,   offline upload of generated document
  * **3-D content import**
    * content in the following formats will be converted to the OpenQwaq internal format upon import
      * 3Ds Max:   .ase,   .obj
      * Google Sketchup:   .kmz
      * VRML2.0
      * openqwaq "point clouds"
    * the Python API can trigger import of most such objects
  * **Pluggable authentication**
    * Some alternate authentication regimes may be supportable by replacing the pluggable authentication module
  * **Server-to-server RFB bridge**
    * the OpenQwaq server can be programmatically configured to offer remote access via RFB to existing desktops



---

## Python API ##

Documentation can be found here
> http://code.google.com/p/openqwaq/source/browse/trunk/docs/OpenQwaq%20Python%20Programmer%20Guide.odt

Some reasons to use the Python API include:

  * You know Python but not Squeak (but note the "not well suited" section below)
  * Python has a robust ecosystem of libraries for interfacing with external systems, and many other purposes
  * API apps can be distributed to users without them needing to update their squeak clients
  * API apps can be installed in a particular forum, so that users entering it experience special behavior
  * API apps can be installed on a particular OpenQwaq server, so that users have access to functionality only while on that server.


Some purposes for which it is not well suited:

  * Changing core functionality, such as document sharing, behavior of panels, avatars, communications, VOIP, etc.
  * High performance
  * High resilience
  * Python-based UI construction


### Samples ###

A zip of samples:  http://code.google.com/p/openqwaq/source/browse/trunk/docs/PythonAPISampleApps.zip

containing:

  * Example applications referred to in programmer's guide
  * OpsCenter.tpz: interactive 3-D datacenter "racks" front-ending external system
  * MakeSticky.tpz: (keyboard controlled for quick creation of multiple notes)
  * StockGraphing.tpz: live data from yahoo stock quotes graphed
  * Gallery.tpz:  interactive 3-D photo gallery

The following applications are built into the client:
  * WhiteBoard
  * FeedReader (RSS viewer)
  * Timer
  * (look in Objects Catalog under "Builtin Apps")


---


For information on the other API's, see the user guide and administrator's guide.