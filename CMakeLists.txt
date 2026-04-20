cmake_minimum_required(VERSION 3.10)

project(MusicPlaylistCore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(Jade
	include/jade/Core.h
	include/jade/Platform.h

	include/jade/App.h
	include/jade/Config.h
	include/jade/Event.h
	include/jade/EventSystem.h
	include/jade/InputSystem.h
	include/jade/MusicLibrary.h
	include/jade/Audio.h

	include/jade/backend/Backend.h
	include/jade/backend/BackendConsole.h

	src/Platform.cpp

	src/App.cpp
	src/EventSystem.cpp
	src/InputSystem.cpp
	src/BackendConsole.cpp
	src/MusicLibrary.cpp
	src/Audio.cpp
)

target_include_directories(Jade PUBLIC
	include
	external/lib/miniaudio
)

add_subdirectory(external/lib/miniaudio)

target_link_libraries(Jade PUBLIC miniaudio)

add_executable(MusicPlaylistTest test/main.cpp)

target_include_directories(MusicPlaylistTest PRIVATE ../include)
target_link_libraries(MusicPlaylistTest PRIVATE Jade)