cmake_minimum_required(VERSION 3.10)

project(MusicPlaylistCore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(MusicPlaylistCore
	include/musicplaylist/Playlist.h
	include/musicplaylist/Generator.h

	src/Playlist.cpp
	src/Generator.cpp
)

target_include_directories(MusicPlaylistCore PUBLIC
	include/
)