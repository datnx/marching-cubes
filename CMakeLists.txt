cmake_minimum_required(VERSION 2.8)

project(marching-cubes)

set(MC_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
set(GLM_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/glm")
set(GLUT_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/freeglut/include")
set(GLEW_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/glew-cmake/include")
set(IMGUI_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/imgui/")

set(MC_SOURCE
	${PROJECT_SOURCE_DIR}/src/main.cpp
	${PROJECT_SOURCE_DIR}/src/shaders.cpp
	${PROJECT_SOURCE_DIR}/src/model.cpp
	${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
	${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glut.cpp
	${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp)

set(MC_HEADERS
	${PROJECT_SOURCE_DIR}/include/shaders.h
	${PROJECT_SOURCE_DIR}/include/model.h
	${PROJECT_SOURCE_DIR}/external/imgui/imgui.h
	${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glut.h
	${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.h)

add_executable(visualize ${MC_SOURCE} ${MC_HEADERS})

add_subdirectory(external/freeglut)

add_subdirectory(external/glew-cmake)

target_include_directories(visualize
	PRIVATE ${MC_INCLUDE_DIRS}
	PRIVATE ${GLM_INCLUDE_DIRS}
	PRIVATE ${GLUT_INCLUDE_DIRS}
	PRIVATE ${GLEW_INCLUDE_DIRS}
	PRIVATE ${IMGUI_INCLUDE_DIRS})

target_link_libraries(visualize freeglut_static libglew_static)

set_property(TARGET visualize PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")