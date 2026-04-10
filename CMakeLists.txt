cmake_minimum_required(VERSION 3.10)

project(MusicPlaylistCore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(MusicPlaylistCore
	include/jade/App.h
	include/jade/Config.h
	include/jade/Event.h
	include/jade/EventSystem.h
	include/jade/MusicDatabase.h
	include/jade/Audio.h

	include/jade/backend/Backend.h
	include/jade/backend/BackendConsole.h

	src/App.cpp
	src/EventSystem.cpp
	src/BackendConsole.cpp
	src/MusicDatabase.cpp
	src/Audio.cpp
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