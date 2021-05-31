project (nn_part C)

set (MAIN host/main.c)
FILE(GLOB SRC host/src/*.c)

add_executable (${PROJECT_NAME} ${SRC} ${MAIN})

target_include_directories(${PROJECT_NAME}
			   PRIVATE host/include
			   PRIVATE ta/include
			   PRIVATE include)

target_link_libraries (${PROJECT_NAME} PRIVATE teec -lm)

install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})