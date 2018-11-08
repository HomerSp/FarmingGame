#include <cstdlib>
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
    if (lodepng::decode(imageData, w, h, buffer) != 0) {
        return -1;
    }

    buffer.clear();

    for (uint32_t i = 0; i < imageData.size(); i += 4) {
        imageData[i] = (imageData[i] < 255 / 2) ? 0 : 255;
        imageData[i + 1] = (imageData[i + 1] < 255 / 2) ? 0 : 255;
        imageData[i + 2] = (imageData[i + 2] < 255 / 2) ? 0 : 255;
        imageData[i + 3] = 255;
    }

    lodepng::State state;
    state.encoder.filter_palette_zero = 0;
    state.encoder.add_id = 0;
    state.encoder.text_compression = 1;
    state.encoder.zlibsettings.nicematch = 258;
    state.encoder.zlibsettings.lazymatching = 1;
    state.encoder.zlibsettings.windowsize = 32768;

    std::vector<unsigned char> temp;
    state.encoder.filter_strategy = LFS_ZERO;
    state.encoder.zlibsettings.minmatch = 3;
    state.encoder.zlibsettings.btype = 2;
    state.encoder.auto_convert = 1;
    if (lodepng::encode(buffer, imageData, w, h, state) != 0) {
        return -1;
    }

    lodepng::save_file(buffer, argv[2]);

    return 0;
}
