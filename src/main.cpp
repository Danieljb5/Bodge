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

    std::vector<BackendGenerator*> generators;
    std::unordered_map<std::string, BackendGenerator*> templateGenerators;
    std::unordered_map<std::string, size_t> templateOutLocations;
    std::unordered_map<std::string, std::vector<std::pair<std::string, bool>>> templateOutputs;
    std::unordered_map<std::string, TemplateSource> templateSources;

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
            generators.push_back(generator);
            parse(generator);

            generator->outSourceFileName = src_dir + "/out/src/" + outFilename + ".c";
            generator->outHeaderFileName = src_dir + "/out/src/" + outFilename + ".h";
            
            generator->outSourceBoilerplateBegin = "#include \"" + outFilename + ".h\"\n";

            generator->outHeaderBoilerplateBegin = "#ifndef " + ToUpper("__" + outFilename + "_H") + "\n";
            generator->outHeaderBoilerplateBegin += "#define " + ToUpper("__" + outFilename + "_H") + "\n";
            generator->outHeaderBoilerplateBegin += "#include \"stdbool.h\"\n";
            generator->outHeaderBoilerplateEnd = "\n#endif // " + ToUpper("__" + outFilename + "_H");
        }
    }

    for(auto& generator : generators)
    {
        auto result = generator->Finish();

        std::ofstream out;
        out.open(generator->outSourceFileName);
        out << generator->outSourceBoilerplateBegin;
        out << result.first;
        out << generator->outSourceBoilerplateEnd;
        out.close();

        out.open(generator->outHeaderFileName);
        out << generator->outHeaderBoilerplateBegin;
        out << result.second;
        out << generator->outHeaderBoilerplateEnd;
        out.close();

        delete generator;
    }
    generators.clear();

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