#include <fstream>
#include <memory>

#include <json/json.h>
#include <json/writer.h>

#include "include/utils.h"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        return -1;
    }

    PtrCompat<char*, 3> args(argv, argc);

    std::ifstream input(args[1]);
    if (!input) {
        return -1;
    }

    Json::Value doc;
    input >> doc;

    Utils::createParentDir(args[2]);

    std::ofstream output(args[2], std::ofstream::out);
    if (!output) {
        return -1;
    }

    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(doc, &output);

    return 0;
}
