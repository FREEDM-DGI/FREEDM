cmake_minimum_required (VERSION 2.6)

project (MingkuiController)

option (ZIGBEE "compile with Zigbee support" ON)

if (ZIGBEE)
  message ("This project is compiled with support for Zigbee."
    "\nChange this setting by running cmake -DZIGBEE=OFF")
endif (ZIGBEE)

configure_file (config.h.cmake config.h )

add_library (socthing SoCObserver)

add_executable (controller DESDbee)

target_link_libraries (controller socthing)

