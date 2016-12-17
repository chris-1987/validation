INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# add STXXL includes path
INCLUDE_DIRECTORIES("${STXXL_INCLUDE_DIRS}")

#build a executable program
ADD_EXECUTABLE(validate validate.cpp)
TARGET_LINK_LIBRARIES(validate ${STXXL_LIBRARIES})

#build a executable program
ADD_EXECUTABLE(validate2 validate2.cpp)
TARGET_LINK_LIBRARIES(validate2 ${STXXL_LIBRARIES})

