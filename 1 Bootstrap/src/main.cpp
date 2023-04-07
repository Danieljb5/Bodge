#include <fstream>
#include <string>

#include "tokens.hpp"

void load_source(const std::string& file, const size_t& _pos = 0);
std::string parse();

std::string boilerplate =
"#include <string>\n"
"using std::string;\n"
"typedef unsigned long size_t;\n";

int main(int argc, char** argv)
{
    if(argc <= 2) return 1;
    load_source(argv[1]);
    std::ofstream out(argv[2]);
    out << boilerplate << parse();
    out.close();
}