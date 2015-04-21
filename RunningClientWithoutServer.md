# Introduction #

There is a little known feature in OpenQwaq which allows one to run the client without the need for a public server. This feature is useful mostly for demoing and debugging.


# Details #

Here is how you enable this:
  * Launch the client
  * Hit the "More Options..." button
  * Hit "+" to add another host
  * Enter "< local >" as custom host (the pair of "< >" triggers it)
  * Hit "Login"

When you've done this, the client will launch a very simple local
server which is sufficient to enter a forum and play with a few
things. It also establishes a connection point which allows other
users to connect to the client if they are on the local network
(although this has not been tested in ages so YMMV). You will not be
able to use apps or documents or any of the more complex backend
services, but you can play with the client and try out some of the
spaces.