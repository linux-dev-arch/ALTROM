#Pleas change NDK path accordingly!!!!

NDK=~/android-ndk-r26c/ #Android NDK install directory
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm #TOOLCHAIN
CLANG=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang

make all TOOLCHAIN=$TOOLCHAIN CC=$CLANG
