#include <cstdlib>
#include <iostream>
#include <fstream>
#include "solver.h"
#include "colors.h"
using namespace std;
using namespace nonogram;

// Constructor
solver::solver(char const *filename)
    : solved_cells_(0)
{
    // Read board dimensions
    read_all_rules(filename);

    // Run sanity checks
    sanity();

    // Generate possible segment patterns
    generate_all_patterns();

    // Set up board
    setup_board();
}

// Run sanity checks
void solver::sanity()
{
    unsigned i, j;
    vector<int> tally;
    tally.resize(color_table_size);
    for (i = 0; i < tally.size(); i++)
    {
        tally[i];
    }

    // Add up all colors in rows
    for (i = 0; i < row_rules_.size(); i++)
    {
        rule_t &cur_rule = row_rules_[i];
        for (j = 0; j < cur_rule.size(); j++)
        {
            rule_element_t &cur = cur_rule[j];
            tally[cur.color_idx] += cur.count;
        }
    }

    // Subtract all colors in cols
    for (i = 0; i < col_rules_.size(); i++)
    {
        rule_t &cur_rule = col_rules_[i];
        for (j = 0; j < cur_rule.size(); j++)
        {
            rule_element_t &cur = cur_rule[j];
            tally[cur.color_idx] -= cur.count;
        }
    }

    // Expect everything to zero out
    for (i = 0; i < tally.size(); i++)
    {
        if (tally[i] != 0)
        {
            printf("Color row/col imbalance - Not solvable\n");
            exit(1);
        }
    }
}

// Run solver
bool solver::run()
{
    size_t old_count;

    // Keep going if we're progressing
    do
    {
        // Save for later
        old_count = solved_cells_;

        // Do a pass
        apply_row_patterns();
        apply_col_patterns();

        // Need to make a guess?
        if (old_count == solved_cells_)
        {
            make_a_guess();
        }

    } while (old_count != solved_cells_);

    return is_solved();
}

// Rule Debugger
void solver::dump_rules()
{
    unsigned row_tally = 0;
    cout << "Rows:" << endl << endl;
    for (unsigned i = 0; i < row_rules_.size(); i++)
    {
        if ((i % 5) == 0)
        {
            cout << endl;
        }
        for (unsigned j = 0; j < row_rules_[i].size(); j++)
        {
            cout << " " << (int)(row_rules_[i][j].count);
            row_tally += row_rules_[i][j].count;
        }
        cout << endl;
    }

    unsigned col_tally = 0;
    cout << endl << "Cols:" << endl << endl;
    for (unsigned i = 0; i < col_rules_.size(); i++)
    {
        if ((i % 5) == 0)
        {
            cout << endl;
        }
        for (unsigned j = 0; j < col_rules_[i].size(); j++)
        {
            cout << " " << (int)(col_rules_[i][j].count);
            col_tally += col_rules_[i][j].count;
        }
        cout << endl;
    }

    cout << "Total row/col sums: " << (int)row_tally << " " << (int)col_tally << endl;
}

bool solver::is_solved()
{
    for (size_t r = 0; r < nrows_; r++)
    {
	for (size_t c = 0; c < ncols_; c++)
	{
	    uint32_t cell = board_[r][c];
	    if (cell & (cell - 1))
	    {
		return false;
	    }
	}
    }
    return true;
}

void solver::apply_row_patterns()
{
    for (size_t r = 0; r < nrows_; r++)
    {
        // Collect OR bitmask of all remaining patterns
        pattern_t rem = row_patterns_[r][0];
        for (size_t id = 1; id < row_patterns_[r].size(); id++)
        {
            pattern_t &cur = row_patterns_[r][id];
            for (size_t c = 0; c < ncols_; c++)
            {
                rem[c] |= cur[c];
            }
        }

        // Use combined possibilities to filter possibilities on board
        size_t old_count = solved_cells_;
        for (size_t c = 0; c < ncols_; c++)
        {
            uint32_t &cell = board_[r][c];
            if (cell != rem[c])
            {
                // Filter
                cell &= rem[c];

                // Check if solved (power of 2)
                if ((cell & (cell - 1)) == 0)
                {
                    solved_cells_++;
                }

                // Sanity check
                if (cell == 0)
                {
                    bail("Puzzle not solvable");
                }
            }
        }

        // progress?
        if (solved_cells_ != old_count)
        {
            prune_col_patterns();
            pause();
        }
    }
}

void solver::apply_col_patterns()
{
    for (size_t c = 0; c < ncols_; c++)
    {
        // Collect OR bitmask of all remaining patterns
        pattern_t rem = col_patterns_[c][0];
        for (size_t id = 1; id < col_patterns_[c].size(); id++)
        {
            pattern_t &cur = col_patterns_[c][id];
            for (size_t r = 0; r < nrows_; r++)
            {
                rem[r] |= cur[r];
            }
        }

        // Use combined possibilities to filter possibilities on board
        size_t old_count = solved_cells_;
        for (size_t r = 0; r < nrows_; r++)
        {
            uint32_t &cell = board_[r][c];
            if (cell != rem[r])
            {
                // Filter
                board_[r][c] &= rem[r];

                // Check if solved (power of 2)
                if ((cell & (cell - 1)) == 0)
                {
                    solved_cells_++;
                }

                // Sanity check
                if (cell == 0)
                {
                    bail("Puzzle not solvable");
                }
            }
        }

        // progress?
        if (solved_cells_ != old_count)
        {
            prune_row_patterns();
            pause();
        }
    }
}

// Discard row patterns that no longer match
void solver::prune_row_patterns()
{
    for (size_t r = 0; r < nrows_; r++)
    {
        std::vector<pattern_t> &pats = row_patterns_[r];

        for (size_t id = 0; id < pats.size(); id++)
        {
            // Does this pattern still match the board?
            pattern_t &cur = pats[id];
            bool consistent = true;
            for (size_t c = 0; c < ncols_; c++)
            {
                if ((cur[c] & board_[r][c]) == 0)
                {
                    consistent = false;
                }
            }

            // If not, cull this pattern and continue
            if (!consistent)
            {
                // Move last element to current spot in vector to
                // avoid a large block copy
                if (id < (pats.size() - 1))
                {
                    pats[id] = pats[pats.size() - 1];
                }
                
                // Erase last element of vector
                pats.erase(pats.begin() + pats.size() - 1);
                id--;
                continue;
            }
        }
    }
}

// Discard column patterns that no longer match
void solver::prune_col_patterns()
{
    for (size_t c = 0; c < ncols_; c++)
    {
        std::vector<pattern_t> &pats = col_patterns_[c];

        for (size_t id = 0; id < pats.size(); id++)
        {
            // Does this pattern still match the board?
            pattern_t &cur = pats[id];
            bool consistent = true;
            for (size_t r = 0; r < nrows_; r++)
            {
                if ((cur[r] & board_[r][c]) == 0)
                {
                    consistent = false;
                }
            }

            // If not, cull this pattern and continue
            if (!consistent)
            {
                // Move last element to current spot in vector to
                // avoid a large block copy
                if (id < (pats.size() - 1))
                {
                    pats[id] = pats[pats.size() - 1];
                }
                
                // Erase last element of vector
                pats.erase(pats.begin() + pats.size() - 1);
                id--;
                continue;
            }
        }
    }
}

void solver::make_a_guess()
{
    // If we're here, the puzzle can't be solved by straight row/column
    // elimination, so we'll pick an arbitrary pattern and see if it works out

    // Find the first row that isn't solved
    size_t r;
    for (r = 0; r < nrows_; r++)
    {
        if (row_patterns_[r].size() != 1)
        {
            break;
        }
    }
    if (r == nrows_)
    {
        // All solved?
        return;
    }

    // Now we're going to try each possible remaining pattern, and find the
    // first which produces a valid result
    for (size_t id = 0; id < row_patterns_[r].size(); id++)
    {
        try
        {
            // Make a clone for scratch space
            solver clone(*this);

            // Stamp current pattern on chosen row in clone
            pattern_t &cur = row_patterns_[r][id];
            for (size_t c = 0; c < cur.size(); c++)
            {
                clone.board_[r][c] = cur[c];
            }

            // See if that worked ...
            if (clone.run())
 	    {
		// Guess it did ...
		*this = clone;
		return;
	    }
        }
        catch (exception &e)
        {
            // That didn't work
        }
    }
}

// Run one pass of solver
void solver::pause()
{
    // Visualize board
    show_board();

    // Track number of patterns in memory
    unsigned long pcount = 0;
    for (size_t r = 0; r < nrows_; r++)
    {
        pcount += row_patterns_[r].size();
    }
    for (size_t c = 0; c < ncols_; c++)
    {
        pcount += col_patterns_[c].size();
    }
    printf("Tracking %lu row/col patterns\n", pcount);

    // Show progress
    double pct = (100. * solved_cells_) / (nrows_ * ncols_);
    printf("%.2f%% complete\n", pct);

    // Wait for user to hit enter
    string foo;
    getline(cin, foo);
}

// Show current state of puzzle
void solver::show_board()
{
    for (size_t r = 0; r < board_.size(); r++)
    {
        for (size_t c = 0; c < board_[r].size(); c++)
        {
            uint32_t &value = board_[r][c];

            // Value a power of two? (identifies single option / solved cell)
            if ((value & (value - 1)) == 0)
            {
                color_print(color_code_by_bitmask(value), "  ");
            }
            else
            {
                // Not solved, put a dot
                color_print("", "::");
            }
        }
        color_print("", "\n");
    }
}

// Emit ANSI console color sequence
void solver::color_print(char const *code, char const *text)
{
    printf("%c[%sm%s", 033, code, text);
}

// Set up puzzle board
void solver::setup_board()
{
    board_.resize(nrows_);
    for (size_t r = 0; r < nrows_; r++)
    {
        board_[r].resize(ncols_);
        for (size_t c = 0; c < ncols_; c++)
        {
            board_[r][c] = -1;
        }
    }
}
        
// Generate all possible patterns for all rows and columns
void solver::generate_all_patterns()
{
    // Generate all row patterns
    for (size_t i = 0; i < nrows_; i++)
    {
        row_patterns_.push_back(generate_patterns(row_rules_[i], ncols_));
    }

    // Generate all column patterns
    for (size_t i = 0; i < ncols_; i++)
    {
        col_patterns_.push_back(generate_patterns(col_rules_[i], nrows_));
    }
}

// Generate all possible patterns for rule
vector<solver::pattern_t> solver::generate_patterns(rule_t &rule, size_t length)
{
    // Start with an empty pattern
    pattern_t empty;
    empty.resize(length);
    for (size_t i = 0; i < length; i++)
    {
        empty[i] = 0x01; // bitmask for empty field (white)
    }

    // Storage for collecting all possible patterns
    vector<pattern_t> possible;

    // Recursively generate all patterns
    recurse_pattern(possible, empty, rule, 0, 0);

    // All done
    return possible;
}

// Recursively generate patterns
void solver::recurse_pattern(vector<pattern_t> &possible, pattern_t ref,
                             rule_t &rule, size_t pat_idx, size_t rule_idx)
{
    // Sanity checks
    if ((pat_idx >= ref.size()) || (rule_idx >= rule.size()))
    {
        return;
    }

    // Ensure at least one pad space between same color segments
    if (rule_idx > 0)
    {
        if (rule[rule_idx-1].color_idx == rule[rule_idx].color_idx)
        {
            // Add pad byte
            pat_idx++;
        }
    }

    // Where can we place the next segment?
    pattern_t cur;
    for (size_t i = pat_idx; i <= (ref.size() - rule[rule_idx].count); i++)
    {
        // Local copy of the reference pattern
        cur = ref;

        // Stencil in this color segment
        for (size_t j = 0; j < (size_t)rule[rule_idx].count; j++)
        {
            cur[i + j] = color_table[rule[rule_idx].color_idx].bitmask;
        }

        // Are we done, or do we recurse?
        if (rule_idx == rule.size() - 1)
        {
            // Done, add to possible patterns
            possible.push_back(cur);
        }
        else
        {
            // Recurse to next segment
            recurse_pattern(possible, cur, rule, i + rule[rule_idx].count, rule_idx+1);
        }
    }
}

// Read all row and column rules
void solver::read_all_rules(char const *filename)
{
    // Open file
    ifstream ifile(filename);
    if (!ifile.good())
    {
        bail("Cannot open file");
    }

    // Get dimensions
    read_dims(ifile);

    // Read row rules
    for (unsigned i = 0; i < nrows_; i++)
    {
        row_rules_.push_back(read_rule(ifile));
    }

    // Read col rules
    for (unsigned i = 0; i < ncols_; i++)
    {
        col_rules_.push_back(read_rule(ifile));
    }
}

// Read puzzle dimensions
void solver::read_dims(ifstream &ifile)
{
    vector<string> tokens = read_tokens(ifile);
    vector<int> dims;

    // Sanity check
    if (tokens.size() != 2)
    {
        bail("Puzzle must have 2 dimensions (first line)");
    }

    // Convert to strings
    for (size_t i = 0; i < tokens.size(); i++)
    {
        dims.push_back(stoi(tokens[i]));

        // Sanity check
        if (dims[i] == 0)
        {
            bail("Puzzle dimensions must be positive");
        }
    }

    // Save results
    ncols_ = dims[0];
    nrows_ = dims[1];
}

// Read row or column rule
solver::rule_t solver::read_rule(ifstream &ifile)
{
    rule_t rules;
    rule_element_t next;

    // Default to black (K)
    next.color_idx = color_table_lookup("K");

    // Read a line of tokens from file
    vector<string> tokens = read_tokens(ifile);

    // Parse
    //cout << "Parsing rule" << endl;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        // Try parsing as an integer
        if (stoi(tokens[i], next.count))
        {
            //cout << "Color idx " << next.color_idx << ", count " << next.count << endl;
            rules.push_back(next);
        }

        // A color code instead?
        else if (color_table_lookup(tokens[i].c_str(), next.color_idx))
        {
            // Assignment done via method call
            //cout << "Switch to color " << next.color_idx << endl;
        }

        // Neither
        else
        {
            cerr << "Unexpected token " << tokens[i] << endl;
            exit(1);
        }
    }

    return rules;
}

// Read tokens from a line in file
vector<string> solver::read_tokens(ifstream &ifile)
{
    // Read a line from file
    string line;
    getline(ifile, line);
    if (!ifile)
    {
        bail("Cannot read from file");
    }

    // Split to tokens
    return split(line);
}

// Split string into tokens by spaces
vector<string> solver::split(std::string data)
{
    vector<string> tokens;
    size_t pos;

    // Separate at spaces
    while ((pos = data.find(" ")) != string::npos)
    {
        // Clip first token
        tokens.push_back(data.substr(0, pos));

        // Repeat with remaining string
        data = data.substr(pos + 1);
    }

    // Add any remaining data
    if (data.size() > 0)
    {
        tokens.push_back(data);
    }

    return tokens;
}

// Convert string to integer (w/ check)
int solver::stoi(std::string &data)
{
    int value;

    if (!stoi(data, value))
    {
        string msg = (string)"Cannot parse " + data + " as integer";
        bail(msg.c_str());
    }

    return value;
}

// Convert string to integer (w/ check)
bool solver::stoi(std::string &data, int &value)
{
    char const *expected_end = data.c_str() + data.size();
    char *actual_end;

    // Do conversion
    value = strtoul(data.c_str(), &actual_end, 10);

    // Make sure entire buffer was parsed
    return (actual_end == expected_end);
}

// Dump an error to screen and stop
void solver::bail(char const *msg)
{
    throw runtime_error(msg);
}
