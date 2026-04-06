cmake_minimum_required(VERSION 3.10)

project(MusicPlaylistCore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(MusicPlaylistCore
	include/musicplaylist/App.h
	include/musicplaylist/Playlist.h
	include/musicplaylist/Generator.h
	include/musicplaylist/Cache.h
	include/musicplaylist/Musicdb.h
	include/musicplaylist/Player.h
	include/musicplaylist/Config.h
	include/musicplaylist/Backend.h
	include/musicplaylist/BackendConsole.h

	src/App.cpp
	src/Playlist.cpp
	src/Generator.cpp
	src/Musicdb.cpp
	src/Player.cpp
	src/Config.cpp
	src/BackendConsole.cpp
)

target_include_directories(MusicPlaylistCore PUBLIC
	include/
	external/lib/miniaudio/
)

add_subdirectory(external/lib/miniaudio)

target_link_libraries(MusicPlaylistCore PUBLIC miniaudio)

add_executable(MusicPlaylistTest test/main.cpp)

target_include_directories(MusicPlaylistTest PRIVATE ../include)
target_link_libraries(MusicPlaylistTest PRIVATE MusicPlaylistCore)