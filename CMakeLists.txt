cmake_minimum_required(VERSION 3.10)

project(doko)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

# Use this for release compilation
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -fomit-frame-pointer")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")

add_executable(doko
               main.cpp
               actual_game_state.cpp
               belief_game_state.cpp
               card_assignment.cpp
               cards.cpp
               game_state.cpp
               game_type.cpp
               human_player.cpp
               move.cpp
               options.cpp
               player.cpp
               random_player.cpp
               rng.cpp
               session.cpp
               timer.cpp
               trick.cpp
               uct.cpp
               uct_player.cpp)
