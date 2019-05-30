#ifndef SOLVER_H
#define SOLVER_H

#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>

namespace nonogram
{

class solver
{
    
    // Storage for row/col rules
    typedef struct
    {
        int color_idx;
        int count;
    } rule_element_t;
    typedef std::vector<rule_element_t> rule_t;

    // Bitmask patterns for each row/col
    typedef std::vector<uint32_t> pattern_t;

public:

    // Constructor
    solver(char const *filename);

    // Implicit destructor
    //~solver();

    // Run sanity checks
    void sanity();

    // Run solver
    bool run();

    // Debugger
    void dump_rules();

    bool is_solved();

private:

    // Solver stages
    void apply_row_patterns();
    void apply_col_patterns();
    void prune_row_patterns();
    void prune_col_patterns();
    void make_a_guess();

    // Stop and display progress
    void pause();

    // Show current state of puzzle
    void show_board();

    // Emit ANSI console color sequence
    void color_print(char const *code, char const *text);

    // Set up puzzle board
    void setup_board();

    // Generate all possible patterns for all rows and columns
    void generate_all_patterns();

    // Generate all possible patterns for rule
    std::vector<pattern_t> generate_patterns(rule_t &rule, size_t length);

    // Recursively generate patterns
    void recurse_pattern(std::vector<pattern_t> &possible, pattern_t ref,
                         rule_t &rule, size_t pat_idx, size_t rule_idx);

    // Read row or column rule
    void read_all_rules(char const *filename);

    // Read puzzle dimensions
    void read_dims(std::ifstream &ifile);

    // Read row or column rule
    rule_t read_rule(std::ifstream &ifile);

    // Read from file as a vector of strings
    std::vector<std::string> read_tokens(std::ifstream &ifile);

    // Split string into tokens by spaces
    std::vector<std::string> split(std::string data);

    // Convert string to integer (exit on fail)
    int stoi(std::string &data);

    // Convert string to integer (return false on failure)
    bool stoi(std::string &data, int &value);

    // Dump an error to screen and stop
    void bail(char const *msg);

    // Dimensions
    size_t nrows_;
    size_t ncols_;

    //
    std::vector<rule_t> row_rules_;
    std::vector<rule_t> col_rules_;

    std::vector<std::vector<pattern_t> > row_patterns_;
    std::vector<std::vector<pattern_t> > col_patterns_;

    // Puzzle board, indexed by rows, then columns
    std::vector<std::vector<uint32_t> > board_;

    // Progress counter (number cells solved)
    size_t solved_cells_;
};

};

#endif
