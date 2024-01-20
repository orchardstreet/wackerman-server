cmake_minimum_required(VERSION 3.0)

project(wackerman_server LANGUAGES C)

add_executable(wackerman_server main.c cmdline.c globals.c multiplexer.c setup.c tools.c)

install(TARGETS wackerman_server RUNTIME DESTINATION bin)
