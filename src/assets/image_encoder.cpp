#include <string>
#include <vector>

#include <lodepng.h>

#include "include/utils.h"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        return -1;
    }

    Utils::createParentDir(argv[2]);

    std::vector<unsigned char> imageData;
    unsigned w, h;
    std::vector<unsigned char> buffer;

    lodepng::load_file(buffer, argv[1]);
    if (lodepng::decode(imageData, w, h, buffer)) {
        return false;
    }

    buffer.clear();

    lodepng::State state;
    state.encoder.filter_palette_zero = 0;
    state.encoder.add_id = false;
    state.encoder.text_compression = 1;
    state.encoder.zlibsettings.nicematch = 258;
    state.encoder.zlibsettings.lazymatching = 1;
    state.encoder.zlibsettings.windowsize = 32768;

    std::vector<unsigned char> temp;
    state.encoder.filter_strategy = LFS_ZERO;
    state.encoder.zlibsettings.minmatch = 3;
    state.encoder.zlibsettings.btype = 2;
    state.encoder.auto_convert = 0;
    if (lodepng::encode(buffer, imageData, w, h, state)) {
        return false;
    }

    lodepng::save_file(buffer, argv[2]);

    return 0;
}
