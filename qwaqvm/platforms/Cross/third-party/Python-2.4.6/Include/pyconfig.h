/* Hand-edited pyconfig.h designed to pull in the correct platform-specific
 * version, depending on platform.
 */
#if defined(_MSC_VER) || defined(WIN32)
# include "pyconfig-win32.h"
#elif __APPLE__ && __MACH__
# include "pyconfig-macosx.h"
#elif __linux__
# include "pyconfig-linux.h"
#else
# error "can't find a suitable pyconfig.h for this platform"
#endif
