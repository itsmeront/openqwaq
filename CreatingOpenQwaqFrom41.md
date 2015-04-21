# Introduction #

The images released initially with OpenQwaq include two different base images. The client image is based on the Croquet Hedgehog SDK release (which is a variant on Squeak 3.8) and the server image is based on Squeak 4.1. This page explains how to transform a Squeak 4.1 image into a suitable OpenQwaq base image.

_NOTE: The first three steps are mostly for documenting the entire process. You can skip straight to step 4 if you use the oqbase image from the [SVN location](http://code.google.com/p/openqwaq/source/browse/#svn%2Ftrunk%2Fimages)._

# Step 1: Unloading everything #

First fetch yourself a Squeak 4.1 image from http://ftp.squeak.org/4.1 to work with. You will need the associated interpreter virtual machine initially, you can _not_ use the OpenQwaq virtual machine quite yet.

Once you have the image, execute the following code:
```
"Unload all packages from 4.1"
Smalltalk unloadAllKnownPackages.

"Get Installer back since it is useful for loading stuff:"
((MCHttpRepository
        location: 'http://www.squeaksource.com/openqwaq'
        user: '' password:'')
        versionFromFileNamed: 'Installer-Core-nice.338.mcz') load.
```

Save your image before you go on to Step 2.

# Step 2: Migrate to the OpenQwaq packages #

In the second step, we migrate all the packages in Squeak to their corresponding OpenQwaq versions:

```
"Prepare class Symbol with AccessProtect class var:"
Symbol addClassVarName: 'AccessProtect'.
Symbol classPool at: 'AccessProtect' put: Mutex new.

"Load the Collections package (proceed through the warning)"
(Installer repository: 'http://www.squeaksource.com/openqwaq') install: 'Collections'.

"Move class Mutex out of the way before loading Kernel"
Mutex rename: #OldMutex.
Mutex category: 'OldMutex'.

"Load the Kernel package (proceed through the warning)"
(Installer repository: 'http://www.squeaksource.com/openqwaq') install: 'Kernel'.

"Update necessary packages."
#(
    SUnit Tests ST80 Balloon Collections CollectionsTests Compiler
    Compression Exceptions Files Graphics GraphicsTests Kernel
    KernelTests Monticello MonticelloConfigurations Morphic
    MorphicExtras MorphicTests Movies Multilingual MultilingualTests
    Network NetworkTests 'PackageInfo-Base' PreferenceBrowser
    SUnitGUI ShoutCore Sound 'Squeak-Version' System
    'ToolBuilder-Kernel' 'ToolBuilder-Morphic' 'ToolBuilder-SUnit'
    Tools ToolsTests TrueType 'XML-Parser'

    "We add these for convenience. The later build process will need them"
    TweakMC 'Qwaq-Build'
) do:[:pkg|
        (Installer repository: 'http://www.squeaksource.com/openqwaq') install: pkg.
].
```

At this point you **must** switch to the OpenQwaq virtual machine. So save your image, and start it again with the OpenQwaq VM.

# Step 3: Image cleanup #
The third step simply cleans up some left-over stuff from the prior steps:

```
"Fix all reference to OldMutex and remove OldMutex package"
(Smalltalk at: #OldMutex)
    allInstancesDo:[:om| om become: Mutex new].
Smalltalk garbageCollect.
(MCWorkingCopy allManagers
    select:[:wc| wc packageName = 'OldMutex']
    thenDo:[:wc| wc unload]).
Smalltalk removeClassNamed: 'OldMutex'.
Smalltalk removeClassNamed: 'MorphObjectOut'.
1 to: 31 do:[:i| 
	(Smalltalk compactClassesArray at: i) ifNotNil:[:aClass|
		aClass isObsolete 
			ifTrue:[Smalltalk compactClassesArray at: i put: nil]]].
Smalltalk recreateSpecialObjectsArray.
SystemOrganization removeEmptyCategories; sortCategories.
DebuggerMethodMap voidMapCache.
Compiler recompileAll.
Smalltalk garbageCollect.
"Condense changes for the base image"
Smalltalk saveAs: 'oqbase.image'.
Smalltalk condenseChanges.
```

At this point you have a suitable OpenQwaq base image that can be used for creating either a client or a server image.

# Step 4: Building a Client image #
With the base image ready and available from the [SVN location](http://code.google.com/p/openqwaq/source/browse/#svn%2Ftrunk%2Fimages), let's create a client image:

```
"Save the base image first"
Smalltalk saveAs: 'oqclient.image'.
"The run the image builder"
QClientImageBuilder run.
```

The resulting image is a **developer** image, not a sealed client image. To prepare a sealed client image, run the following code:

```
"Set the client version properly (replace x.y.zz with your version)"
QwaqVersion newVersion: 'OpenQwaq x.y.zz'.
"Save the image under the sealed image name"
Smalltalk saveAs: 'OpenQwaq.image'.
"Set the interrupt password"
QwaqForumsUI interruptPassword: ''.
"And run the client as standalone app"
WorldState addDeferredUIMessage:
	(MessageSend
		receiver: QwaqForumsUI
		selector: #runAsApp:
		argument: false).
```

# Step 5: Building a Server image #
The server image build process is almost identical to the client image build process. It starts with an oqbase.image from the [SVN location](http://code.google.com/p/openqwaq/source/browse/#svn%2Ftrunk%2Fimages) and then loads the required server packages:

```
"Save the base image first"
Smalltalk saveAs: 'oqserver.image'.
"The run the image builder"
QServerImageBuilder run.
```

Again, the resulting image is a **developer** image, not a sealed server image. To seal the server image execute the following:

```
"Set the client version properly (replace x.y.zz with your version)"
QwaqVersion newVersion: 'Server x.y.zz'.
"Save the image under the sealed image name"
Smalltalk saveAs: 'server.image'.
"Set the interrupt password"
QwaqServerUI interruptPassword: ''.
"And run the server as standalone app"
WorldState addDeferredUIMessage:
	(MessageSend
		receiver: QwaqServerUI
		selector: #runAsApp:
		argument: false).
```

# Step 6: Keeping the images up-to-date #
Once you have a developer image (either oqclient.image or oqserver.image), you can also incrementally update it instead of rebuilding it. In order to update all the packages in your image, evaluate the following:

```
"Update all packages"
UsefulScripts updateAllLoadedPackages
```

This will update all the loaded packages in your image to the latest versions in the repository. The image can then be sealed again for deployment by executing the code shown above.