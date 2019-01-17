include_directories(../)

add_executable(example1 example1.cc)
target_link_libraries(example1 -lpthread)

#add_executable(example2 example2.cc)
#target_link_libraries(example2 -lpthread)

add_executable(example3 example3.cc)
target_link_libraries(example3 -lpthread)
