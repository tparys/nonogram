add_library(game solver.cpp colors.cpp)

add_executable(nonogram main.cpp)
target_link_libraries(nonogram game)

add_executable(debugger debugger.cpp)
target_link_libraries(debugger game)
