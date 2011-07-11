###############################################################################
# @file      CMakeLists.txt
#
# @author    Derek Ditch <derek.ditch@mst.edu>
#
# @compiler  CMake 2.8
#
# @project   FREEDM DGI
#
# @description  Build script for unit tests.
#
# These source code files were created at as part of the
# FREEDM DGI Subthrust, and are
# intended for use in teaching or research.  They may be 
# freely copied, modified and redistributed as long
# as modified versions are clearly marked as such and
# this notice is not removed.
# 
# Neither the authors nor the FREEDM Project nor the
# National Science Foundation
# make any warranty, express or implied, nor assumes
# any legal responsibility for the accuracy,
# completeness or usefulness of these codes or any
# information distributed with these codes.
#
# Suggested modifications or questions about these codes 
# can be directed to Dr. Bruce McMillin, Department of 
# Computer Science, Missour University of Science and
# Technology, Rolla, MO  65409 (ff@mst.edu).
#
###############################################################################


set (depends ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})

MACRO(BROKER_ADD_TEST)
  PARSE_ARGUMENTS(TEST
    "LINK_LIBRARIES;DEPENDS"
    ""
    ${ARGN}
    )
  CAR(TEST_NAME ${TEST_DEFAULT_ARGS})
  CDR(TEST_SOURCES ${TEST_DEFAULT_ARGS})

  add_executable( ${TEST_NAME} ${TEST_SOURCES} )

  target_link_libraries( ${TEST_NAME} ${TEST_LINK_LIBRARIES} )

  add_test( ${TEST_NAME} ${TEST_NAME} )

ENDMACRO(BROKER_ADD_TEST)

#foreach(test
#    header_compiler_test
#    cmessage_test
#   )
#  broker_add_test(${test} LINK_LIBRARIES ${depends} )
#endforeach()

broker_add_test( test_header_compile test_header_compile.cpp
    LINK_LIBRARIES ${Boost_SYSTEM_LIBRARY} )
broker_add_test( test_cmessage test_cmessage.cpp ../src/CMessage.cpp
    LINK_LIBRARIES ${Boost_SYSTEM_LIBRARY} )
broker_add_test( test_requestparser test_requestparser.cpp
    ../src/CMessage.cpp
    LINK_LIBRARIES ${Boost_SYSTEM_LIBRARY} )
broker_add_Test( test_cconnection test_cconnection.cpp ../src/CConnection.cpp
    ../src/CDispatcher.cpp ../src/CConnectionManager.cpp ../src/CMessage.cpp
    LINK_LIBRARIES ${Boost_SYSTEM_LIBRARY} )
    
broker_add_test( test_cdispatch test_cdispatch.cpp ../src/CDispatcher.cpp
    ../src/CMessage.cpp
    LINK_LIBRARIES ${Boost_SYSTEM_LIBRARY} )
    
broker_add_Test( test_uuid test_uuid.cpp )



