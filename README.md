# MdnsdTest
Mdns test for Android

Linux:

1. cd build.android

2. ndk-build

3. adb push libs/arm64-v8a/test /data/tmp

Android:

1. adb root

2. adb shell

3. cd data/tmp

4. ./test
