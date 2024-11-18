


#comparison
add_library(comparison src/ltl-compare.cc)
target_include_directories(comparison PUBLIC include/ ${SPOTLTLCPP_INCLUDE_DIRS})


###Executable---------------------------------###
add_executable(ltl-compare EXCLUDE_FROM_ALL src/main.cc) 
target_include_directories(ltl-compare PUBLIC include/)
set_target_properties(ltl-compare PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

#link harm libs
target_link_libraries(ltl-compare traceReader temporalParser propositionParser)

#link command line handler lib
target_link_libraries(ltl-compare commandLineParser comparison ${SPOTLTL_LIB})

