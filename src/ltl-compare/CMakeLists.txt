
#dea

add_executable(ltl-compare EXCLUDE_FROM_ALL src/main.cc)
set_target_properties(ltl-compare PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

#SET(LTL_COMPARE_SRC src/)
#add_library(evaluator ${LTL_COMPARE_SRC} src/.cc)
target_include_directories(ltl-compare PUBLIC include/)

#link harm libs
target_link_libraries(ltl-compare traceReader temporalParser propositionParser)

#link command line handler lib
target_link_libraries(ltl-compare commandLineParser)
