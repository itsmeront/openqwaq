# Procedure for creating OpenQwaq Client for OSX developers with Open Source Video Codecs #

I would like to share a step-by-step description of:
- How I created an OpenQwaq Client bundle for OSX (unsealed image), with the open source video and audio codecs included.
- How I launched an OpenQwaq Forum and tested the webcam functionality

This should also provide a basis for creating both client and server OpenQwaq bundles for other platforms, specifically Linux and Win.

Hoping this helps, and any feedback would be very welcome.
Happy New Year,
Reza

---

Here is the procedure for creating and testing the bundle:

1) From http://code.google.com/p/openqwaq/downloads/list
> downloaded the "OpenQwaq 1.0.01-mac.zip" archive (OpenQwaq Client Mac).

2) Unzipped (1) in a folder named "OpenQwaq 1.0.01".

3) Following the instructions in http://code.google.com/p/openqwaq/wiki/CreatingOpenQwaqFrom41
> created an up-to-date OpenQwaq-Client image (starting from Squeak 4.1).

NOTE: the "image" obtained in (2) does not include the code that integrates QVideoCodecPluginFree.bundle. Hence the necessity for this step (3).

4) Set the version ID for the image created in (3) to 'OpenQwaq 1.0.02.3', by executing the following in a workspace:
> QwaqVersion current version: 'OpenQwaq 1.0.03'.

NOTE: The default version is: 'OpenQwaq anonymous build'.

5) Stored the bundle obtained in (3) and (4) in a folder named "OpenQwaq 1.0.03".

6) Following the instructions in http://code.google.com/p/openqwaq/source/checkout,
> checked out a read-only working copy of the OpenQwaq sources.

7) Unzipped (5) in a folder named "OpenQwaq-CVS".

8) From "OpenQwaq-CVS/qwaqvm-binary/mac", duplicated "OpenQwaq" to a new folder named "OpenQwaq 1.0.05".

9) Copied into "OpenQwaq 1.0.05/Contents/Resources" the followings:
> -"OpenQwaq-CVS/client/icones"
> -"OpenQwaq-CVS/client/Objects"
> -"OpenQwaq-CVS/client/objects-menu"
> -"OpenQwaq-CVS/client/ports.txt"
> -"OpenQwaq-CVS/client/scripts"

> - "OpenQwaq 1.0.03/oqclient.image"
> - "OpenQwaq 1.0.03/oqclient.changes"
> - "OpenQwaq 1.0.03/SqueakV41.sources"

10) Copied "OpenQwaq 1.0.01/Contents/Info.plist" into "OpenQwaq 1.0.05/Contents".

11) Edited the following entries in "OpenQwaq 1.0.05/Contents/Info.plist":
> - "CFBundleGetInfoString" changed from "OpenQwaq 1.0.01" to "OpenQwaq 1.0.05"
> - "CFBundleShortVersionString" changed from "OpenQwaq 1.0.01" to "OpenQwaq 1.0.05"
> - "CFBundleVersion" changed from "OpenQwaq 1.0.01" to "OpenQwaq 1.0.05"
> - "SqueakImageName" changed from "OpenQwaq.image" to "oqclient.image"

12) Launched "OpenQwaq 1.0.05".

13) Set the version ID of its image (copied in (9) from "OpenQwaq 1.0.03") to 'OpenQwaq 1.0.05', by executing the following in a workspace:
> QwaqVersion current version: 'OpenQwaq 1.0.05'.

14) Tested the following default directories:
> - Smalltalk vmPath.
> - Smalltalk getSystemAttribute:0.
> - FileDirectory cacheDirectory.

15) Saved the image ("OpenQwaq 1.0.05").

16) Opened the "Welcome to OpenQwaq" dialog by executing the followings in a workspace:

> - QForms current importDirectory: 'icons'.
> - QForms current importDirectory: 'icons/Qwaq'.
> - QForms current importDirectory: 'icons/OpenQwaq'.
> - QForms current importDirectory: 'icons/Qwaq/Cards'.
> - QForms current importDirectory: 'icons/Qwaq/splash'.
> - CProjectMorph open: QwaqParticipantUI.

NOTE: Alternative Forums are proposed by the followings:
> - CProjectMorph open: QJasmineForumDemo.
> - CProjectMorph open: QwaqForumsUI.

17) In the "Welcome to OpenQwaq" dialog:
> - Clicked on the "Proxy" button to open the "Proxy Configuration" dialog.
> - Made sure that the selected option was "Direct connection (no proxy)".
> - Pushed the "Accept" button, which returned me back to the "Welcome to OpenQwaq" dialog.

18) Pushed the "Login" button in the "Welcome to OpenQwaq" dialog, which popped the "OpenQwaq Lobby".

19) In the "OpenQwaq Lobby", selected a Forum, and pushed the "Enter Forum" button.

20) Tested the open source video codecs by pushing the Camera button, which successfully turned the Camera on....