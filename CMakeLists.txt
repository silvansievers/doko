cmake_minimum_required(VERSION 3.10)

project(doko)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# TODO: use this for release build
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -fomit-frame-pointer")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")

set(
     CC_FILES
          actual_game_state
          belief_game_state
          card_assignment
          cards
          game_state
          game_type
          human_player
          move
          options
          player
          random_player
          rng
          session
          timer
          trick
          uct
          uct_player)

set(SOURCES)
foreach(FILE ${CC_FILES})
    list(APPEND SOURCES main.cpp ${FILE}.cpp ${FILE}.h)
endforeach()

add_executable(hug ${SOURCES})
