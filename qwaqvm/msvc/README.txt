
     The "msvc" directory contains the environment files for building
the Croquet virtual machine on Microsoft Windows, with Visual Studio 8
or later (e.g., Visual Studio Express 2005, downloadable at no charge
from Microsoft).

     Before launching Visual Studio, use the "System" Windows control
panel to set an environment variable "squeakvm", with the root
directory of your VM tree as the value. The Visual Studio project
files use this variable to express include-directory paths. Place the
"msvc" directory as a child of that root directory, and a sibling to
the "platforms" and "winbuild" directories from squeakvm.org.

     The "winbuild" directory contains both build environment files
(for the MinGW tools) and source code files. The maintainer of Squeak
on Windows will probably want to integrate the contents of the "msvc"
directory with the "winbuild" directory; there's no need for two
directories.


     Craig Latta <craig@netjam.org>
     2008-05-27

