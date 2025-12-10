add_executable(hardBrainTest hardBrainTest.c)
target_link_libraries(hardBrainTest PUBLIC btidg2 m)

find_package(Python)

if(Python_FOUND)
   
	execute_process(COMMAND ${Python_EXECUTABLE} ${CMAKE_SOURCE_DIR}/translator/converter.py ${CMAKE_SOURCE_DIR}/translator/Configs/littleConfigTest.json)

    add_executable(littleConfigTest littleConfigTest.c)
    target_link_libraries(littleConfigTest PUBLIC btidg2 m)
    target_include_directories(littleConfigTest PUBLIC ${CMAKE_SOURCE_DIR}/translator/Configs)
else()
	message("Python is required for example littleConfigTest.c")
endif()

find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()
