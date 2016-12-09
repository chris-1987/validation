INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# add stxxl includes path
INCLUDE_DIRECTORIES("${STXXL_INCLUDE_DIRS}")

add_subdirectory(methodA)

add_subdirectory(methodB)



