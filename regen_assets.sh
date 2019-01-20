cd tools &&
cmake . &&
cmake --build . &&
cd .. &&

./tools/font-packer /usr/share/fonts/noto/NotoSans-Regular.ttf /usr/share/fonts/noto-cjk/NotoSansCJKjp-Regular.otf shaders/chars.txt &&
mv font_pack_meta.dat data/ &&
mv fonts_bitmap.myyraw textures/ &&

ruby tools/ShadersPacker.rb shaders/metadata.ini &&
mv shaders/shaders.h ./ &&
mv shaders/shaders.pack data/

