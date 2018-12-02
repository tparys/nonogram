#include <cstdlib>
#include <iostream>
#include "solver.h"
using namespace std;
using namespace nonogram;

int main(int argc, char **argv)
{
    // Sanity check
    if (argc < 2)
    {
        cerr << "No input file specified" << endl;
        exit(1);
    }

    solver app(argv[1]);
    app.dump_rules();

    return 0;
}
