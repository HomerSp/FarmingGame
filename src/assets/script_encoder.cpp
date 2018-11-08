#include <fstream>

#include "include/utils.h"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        return -1;
    }

    PtrCompat<char*, 3> args(argv, argc);

    Utils::createParentDir(args[2]);

    std::ifstream src(args[1], std::ios::binary);
    std::ofstream dst(args[2], std::ios::binary);

    dst << src.rdbuf();

    return 0;
}
