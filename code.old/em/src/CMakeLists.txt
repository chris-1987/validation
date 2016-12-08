INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# add STXXL includes path
INCLUDE_DIRECTORIES("${STXXL_INCLUDE_DIRS}")

#build a executable program
ADD_EXECUTABLE(lcpa_em lcpa_em.cpp)
TARGET_LINK_LIBRARIES(lcpa_em ${STXXL_LIBRARIES})

#build a executable program
ADD_EXECUTABLE(lcpa_em2 lcpa_em2.cpp)
TARGET_LINK_LIBRARIES(lcpa_em2 ${STXXL_LIBRARIES})

##build a executable program
#ADD_EXECUTABLE(klcpa_em klcpa_em.cpp)
#TARGET_LINK_LIBRARIES(klcpa_em ${STXXL_LIBRARIES})

#build a executable program
#ADD_EXECUTABLE(klcpa_em_opt klcpa_em_opt.cpp)
#TARGET_LINK_LIBRARIES(klcpa_em_opt ${STXXL_LIBRARIES})
