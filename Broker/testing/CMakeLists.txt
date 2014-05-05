add_executable(test-serial test-serial.cpp)

# link the executable to its dependencies
target_link_libraries(test-serial
                      broker
                      device
                      ${Boost_DATE_TIME_LIBRARY}
                      ${Boost_PROGRAM_OPTIONS_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_THREAD_LIBRARY}
                     )
