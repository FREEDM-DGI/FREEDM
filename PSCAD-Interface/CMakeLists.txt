# CMake specification for PSCAD-Interface
cmake_minimum_required (VERSION 2.6)
project (PSCAD_INTERFACE)

# find the boost libraries required for this project
find_package (Boost REQUIRED COMPONENTS system thread program_options)

if (Boost_FOUND)
    # add the found libraries to the project
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    # goto src/CMakeLists.txt
    add_subdirectory (src)
endif (Boost_FOUND)
