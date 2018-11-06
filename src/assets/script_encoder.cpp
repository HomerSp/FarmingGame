#include <fstream>

#include "include/utils.h"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        return -1;
    }

    Utils::createParentDir(argv[2]);

    std::ifstream src(argv[1], std::ios::binary);
    std::ofstream dst(argv[2], std::ios::binary);

    dst << src.rdbuf();

    return 0;
}
