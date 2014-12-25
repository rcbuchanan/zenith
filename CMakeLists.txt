cmake_minimum_required (VERSION 2.6)
project (zenith)

set (zenith_VERSION_MAJOR 0)
set (zenith_VERSION_MINOR 1)

set(CMAKE_C_FLAGS "-pedantic -Wall -std=c99")

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

configure_file (
	"${PROJECT_SOURCE_DIR}/zenith_config.h.in"
	"${PROJECT_BINARY_DIR}/zenith_config.h"
)

include_directories("${PROJECT_BINARY_DIR}")


#pthread -lm -lGL -lGLU -lGLEW -lglut -lgsl -lgslcblas -Wall -lpng -pedantic -std=c99 && ./a.out

add_executable(zenith
	axis.c
	filewatcher.c
	landscape.c
	matstack.c
	skybox.c
	tga.c
	util.c
	view.c
	zenith.c
)

target_link_libraries(zenith
	m
	${OPENGL_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	${GLEW_LIBRARY}
	${GLUT_LIBRARY}
)
