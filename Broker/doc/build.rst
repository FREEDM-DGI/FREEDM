====================================
 Build Instructions
====================================
I recommend the usual CMake way to use out-of-source builds. This makes it very easy to keep
the source tree clean. If you wish to revert to just the source (say before a svn checkin),
just remove the build directory. You could also use different build directories for different
build types. Perhaps one which is a debug build, one which is a release build.

Requirements
-----------------
Broker requires the Boost libraries version 1.42 or greater.


GNU Makefile Generation
------------------------------------
Below, I assume you create a directory called 'build' in the top-level project directory.

        mkdir build && cd build
        cmake -DCMAKE_BUILD_TYPE=Debug ../


Eclipse CDT4
------------------------------------
Use the Eclipse CDT4 generator to generate project files. Due to the differences in the way Eclipse 
performs builds (within the source) and CMake builds projects (outside the source), you'll want to
create two projects. One will manage source files, while the other manages the build. Create a 
sibling directory to `Broker`, say `BrokerBuild` and issue the following commands:

	cd BrokerBuild
	cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug \
		-DECLIPSE_CDT4_GENERATE_SOURCE_PROJECT=TRUE ../Broker
		
This will create (or update) a .project file in the `Broker` directory and corresponding project
settings in the `BrokerBuild` directory. Import both of these projects into the workspace and
edit the files in the Broker project while executing builds from the BrokerBuild project.
