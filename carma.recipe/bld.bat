cmake -S . -B build ^
      -G "Visual Studio 17 2022" ^
      -DCMAKE_BUILD_TYPE:STRING=Release ^
      -DCMAKE_INSTALL_PREFIX:PATH=%PREFIX%
cmake --build build --target ALL_BUILD --config Release
cmake --install build