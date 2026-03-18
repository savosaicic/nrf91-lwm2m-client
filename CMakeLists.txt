cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(nrf91_lwm2m_client)

target_include_directories(app PRIVATE
  include
  ${ZEPHYR_BASE}/subsys/net/lib/lwm2m
)
target_sources(app PRIVATE src/main.c src/temperature.c src/time.c src/lwm2m_obj_time.c)
