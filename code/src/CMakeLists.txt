INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# add STXXL includes path
INCLUDE_DIRECTORIES("${STXXL_INCLUDE_DIRS}")

#build the executable for validate
ADD_EXECUTABLE(validate validate.cpp)
TARGET_LINK_LIBRARIES(validate ${STXXL_LIBRARIES})

#build the executable for validate2 (internal sort + Karp rabin)
ADD_EXECUTABLE(validate2 validate2.cpp)
TARGET_LINK_LIBRARIES(validate2 ${STXXL_LIBRARIES})


#build the executable for validate3 (external sort + Karp rabin)
ADD_EXECUTABLE(validate3 validate3.cpp)
TARGET_LINK_LIBRARIES(validate3 ${STXXL_LIBRARIES})

