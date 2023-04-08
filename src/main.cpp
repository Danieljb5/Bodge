#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <string>

#include "cbackend.hpp"
#include "tokens.hpp"

void load_source(const std::string& file, const size_t& _pos = 0);
void parse(BackendGenerator* generator);

std::string ToUpper(const std::string& str)
{
    std::string result = str;
    for(auto& c : result)
    {
        c = toupper(c);
    }
    return result;
}

int main(int argc, char** argv)
{
    if(argc <= 2) return 1;
    std::string src_dir = argv[1];
    std::string output = argv[2];

    std::experimental::filesystem::create_directories(src_dir + "/out/src");
    std::experimental::filesystem::create_directories(src_dir + "/out/obj");

    for(auto& f : std::experimental::filesystem::recursive_directory_iterator(src_dir))
    {
        if(f.path().extension() == ".b")
        {
            std::cout << "processing " << f.path().filename() << "\n";
            std::string path = f.path().filename().string();
            load_source(src_dir + "/" + path);

            std::string outFilename = path;
            auto pos = outFilename.find_last_of('.');
            outFilename = outFilename.substr(0, pos);

            BackendGenerator* generator = new CBackendGenerator();
            parse(generator);

            std::ofstream out(src_dir + "/out/src/" + outFilename + ".c");
            auto result = generator->Finish();
            out << "#include \"" << outFilename << ".h\"\n" << result.first;
            out.close();

            out.open(src_dir + "/out/src/" + outFilename + ".h");
            out << "#ifndef " << ToUpper("__" + outFilename + "_H")
            << "\n#define " << ToUpper("__" + outFilename + "_H")
            << "\n" << result.second
            << "\n#endif //" << ToUpper("__" + outFilename + "_H");
            out.close();

            delete generator;
        }
    }

    for(auto& f : std::experimental::filesystem::recursive_directory_iterator(src_dir + "/out/src"))
    {
        if(f.path().extension() == ".c")
        {
            std::cout << "compiling " << f.path().filename() << "\n";
            std::string outFilename = f.path().filename();
            auto pos = outFilename.find_last_of('.');
            outFilename = outFilename.substr(0, pos);

            system(("gcc -c " + src_dir + "/out/src/" + outFilename + ".c -o " + src_dir + "/out/obj/" + outFilename + ".o -I" + src_dir + "/out/").c_str());
        }
    }

    system(("gcc " + src_dir + "/out/obj/*.o -o " + src_dir + "/out/" + output).c_str());
}