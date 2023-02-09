add_executable(BTIG2_A BTIG2_adjacency.c)
add_executable(BTIG2_A2 BTIG2_adjacency2.c)
#Maths
target_link_libraries(BTIG2_A PRIVATE m)
target_link_libraries(BTIG2_A2 PRIVATE m)

# MPI
find_package(MPI REQUIRED)
target_link_libraries(BTIG2_A PUBLIC MPI::MPI_CXX)
target_link_libraries(BTIG2_A2 PUBLIC MPI::MPI_CXX)

