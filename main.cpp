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

    try
    {
        if (app.run())
        {
            cout << "Solution found" << endl;
        }
        else
        {
            cout << "No solution found" << endl;
        }
    }
    catch (exception &e)
    {
        cerr << "Caught an exception: " << e.what() << endl;
    }

    return 0;
}
