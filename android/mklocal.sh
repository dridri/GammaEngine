#!/bin/bash

ndk-build -j8 $@

sudo cp -v obj/local/armeabi/libgammaengine.a /opt/android-ndk-r10d/platforms/android-15/arch-arm/usr/lib/
sudo cp -v obj/local/arm64-v8a/libgammaengine.a /opt/android-ndk-r10d/platforms/android-15/arch-arm64/usr/lib/
sudo cp -v obj/local/mips/libgammaengine.a /opt/android-ndk-r10d/platforms/android-15/arch-mips/usr/lib/
sudo cp -v obj/local/mips64/libgammaengine.a /opt/android-ndk-r10d/platforms/android-15/arch-mips64/usr/lib/
sudo cp -v obj/local/x86/libgammaengine.a /opt/android-ndk-r10d/platforms/android-15/arch-x86/usr/lib/
sudo cp -v obj/local/x86_64/libgammaengine.a /opt/android-ndk-r10d/platforms/android-15/arch-x86_64/usr/lib/

sudo cp -v obj/local/armeabi/libgammaengine.a /opt/android-ndk-r10d/platforms/android-21/arch-arm/usr/lib/
sudo cp -v obj/local/arm64-v8a/libgammaengine.a /opt/android-ndk-r10d/platforms/android-21/arch-arm64/usr/lib/
sudo cp -v obj/local/mips/libgammaengine.a /opt/android-ndk-r10d/platforms/android-21/arch-mips/usr/lib/
sudo cp -v obj/local/mips64/libgammaengine.a /opt/android-ndk-r10d/platforms/android-21/arch-mips64/usr/lib/
sudo cp -v obj/local/x86/libgammaengine.a /opt/android-ndk-r10d/platforms/android-21/arch-x86/usr/lib/
sudo cp -v obj/local/x86_64/libgammaengine.a /opt/android-ndk-r10d/platforms/android-21/arch-x86_64/usr/lib/

sudo mkdir -p /opt/android-ndk-r10d/platforms/android-15/arch-arm/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-15/arch-arm64/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-15/arch-mips/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-15/arch-mips64/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-15/arch-x86/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-15/arch-x86_64/usr/include/gammaengine/

sudo mkdir -p /opt/android-ndk-r10d/platforms/android-21/arch-arm/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-21/arch-arm64/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-21/arch-mips/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-21/arch-mips64/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-21/arch-x86/usr/include/gammaengine/
sudo mkdir -p /opt/android-ndk-r10d/platforms/android-21/arch-x86_64/usr/include/gammaengine/

sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-15/arch-arm/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-15/arch-arm64/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-15/arch-mips/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-15/arch-mips64/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-15/arch-x86/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-15/arch-x86_64/usr/include/gammaengine/

sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-21/arch-arm/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-21/arch-arm64/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-21/arch-mips/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-21/arch-mips64/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-21/arch-x86/usr/include/gammaengine/
sudo cp -v ../include/*.h ../include/android/*.h ../src/Vector.cpp /opt/android-ndk-r10d/platforms/android-21/arch-x86_64/usr/include/gammaengine/
