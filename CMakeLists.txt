cmake_minimum_required(VERSION 3.10)
project(ComputerAnimation CXX)

# Find includes in corresponding build directories
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(OpenGL_GL_PREFERENCE "GLVND")

include_directories("${PROJECT_SOURCE_DIR}/deps/include")

add_library(AnimLib
	src/Camera.cpp
	src/Camera.h
	src/Shader.cpp
	src/Shader.h
	src/Texture.cpp
	src/Texture.h
	src/Window.cpp
	src/Window.h
	src/Model.cpp
	src/Model.h
	src/utils/File.h
	src/utils/Logger.h
	src/VideoPlayer.cpp
	src/VideoPlayer.h)
add_executable(${PROJECT_NAME} main.cpp)

find_package(OpenGL REQUIRED)
find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(LIBS OpenGL::GL)

if(WIN32)
	file(GLOB DLLS "${PROJECT_SOURCE_DIR}/deps/dll/*")
	file(COPY ${DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

	file(GLOB WINLIBS "${PROJECT_SOURCE_DIR}/deps/lib/*")
	message(${WINLIBS})
	foreach(LIB ${WINLIBS})
		set(LIBS ${LIBS} ${LIB})
	endforeach()
else()
	find_package(GLEW REQUIRED)
	find_package(glfw3 CONFIG REQUIRED)
	find_package(glm CONFIG REQUIRED)
	find_package(FreeImage REQUIRED)
	find_package(OpenCV REQUIRED)
	
	include_directories("${PROJECT_SOURCE_DIR}/src" ${ASSIMP_INCLUDE_DIRS})
	
	set(LIBS
		${LIBS}
		GLEW::GLEW
		glfw
		glm
		${FREEIMAGE_LIBRARY}
		${ASSIMP_LIBRARIES})

	target_include_directories(AnimLib PRIVATE ${OpenCV_INCLUDE_DIRS})
	target_link_libraries(AnimLib PRIVATE ${OpenCV_LIBS})

	target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
endif()

target_link_libraries(AnimLib PRIVATE ${LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL ${LIBS} AnimLib)

if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND if not exist \"${CMAKE_CURRENT_BINARY_DIR}/Data\"
		mklink /d
		\"${CMAKE_CURRENT_BINARY_DIR}/Data\"
		\"${PROJECT_SOURCE_DIR}/Data\"
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/Data"
	)

	# set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/PROFILE")
else()
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND}
		-E
		create_symlink "${PROJECT_SOURCE_DIR}/Data"
		"${CMAKE_CURRENT_BINARY_DIR}/Data"
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/Data"
	)
endif()
