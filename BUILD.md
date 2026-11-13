# Сборка Tool1CD

Проект поддерживает CMake 3.10 и выше. Необходим компилятор C++ с поддержкой C++11.

Необходимые библиотеки:
  * Boost.FileSystem (>= 1.53)
  * Boost.Regex (>= 1.53)
  * Boost.UUID (>= 1.53)
  * Boost.CRC (>= 1.53)
  * Boost.IOStreams (>= 1.53)
  * zlib (>= 1.2.8)

## Ubuntu

```sh
# Установка системы сборки:
sudo apt-get install cmake make g++ libstdc++-dev

# Установка зависимостей:
sudo apt-get install libboost-filesytem-dev libboost-system-dev libboost-regex-dev zlib1g-dev

# Сборка:
mkdir build && cd build
cmake .. && make
make install # или checkinstall
```

Сборка deb-пакета:

```sh
sudo apt-get install debhelper
debuild -uc -us
```

Поддержка разработки графического интерфейса пользователя.

```sh
sudo apt install mesa-common-dev
```

Скачать и установить скомпилированные библиотеки [Qt](https://www.qt.io/download)

deb-пакет будет в каталоге уровнем выше с названием `ctool1cd_<версия>_<платформа>.deb`

## Windows

Нужны Visual Studio 2022 с рабочей нагрузкой «Разработка классических приложений на C++» (в комплекте есть CMake) и [vcpkg](https://github.com/microsoft/vcpkg). CI проверяет сборку на теге vcpkg `2026.10.16`.

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
git -C C:\vcpkg checkout 2026.10.16
C:\vcpkg\bootstrap-vcpkg.bat
```

Каталоги `C:\vcpkg` и `C:\Qt\6.8.3\msvc2022_64` ниже — примеры. Подставьте свои пути.

Сборка консоли вместе с GUI и сборка только консоли конфигурируются отдельно: у них разные triplet vcpkg и разный runtime CRT. Для каждой нужен свой каталог `build`.

### Консоль и GUI

Так проект собирается в CI. Готовые библиотеки Qt — динамические (`/MD`), поэтому triplet vcpkg тоже динамический: `x64-windows`. `NOGUI` не задаётся.

Зависимости vcpkg:

```powershell
C:\vcpkg\vcpkg.exe install --triplet x64-windows `
  boost-filesystem boost-system boost-regex boost-random `
  boost-uuid boost-crc boost-algorithm zlib vulkan
```

[Qt 6.8.3](https://www.qt.io/download), комплект **MSVC 2022 64-bit**, модуль **Qt 5 Compatibility** (`Core5Compat`). В CI используется именно 6.8.3.

[Vulkan SDK](https://vulkan.lunarg.com/sdk/home) версии **1.4.309.0**. Перед конфигурацией переменная `VULKAN_SDK` должна указывать на каталог установки, обычно `C:\VulkanSDK\1.4.309.0`.

```powershell
cd C:\path\to\tool1cd
mkdir build
cd build
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.309.0"
cmake -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64 `
  ..
cmake --build . --config Release --target ctool1cd --target testproject --target gtool1cd
```

Готовые файлы: `build\bin\Release\ctool1cd.exe`, `build\bin\Release\testproject.exe`, `build\bin\Release\gtool1cd.exe`.

Чтобы GUI запускался рядом со своими DLL Qt:

```powershell
$env:PATH = "C:\Qt\6.8.3\msvc2022_64\bin;$env:PATH"
windeployqt --release --no-compiler-runtime --no-translations `
  --no-system-d3d-compiler --no-opengl-sw --no-patchqt `
  .\bin\Release\gtool1cd.exe
```

Открыть каталог в Visual Studio можно через `Файл → Открыть → Папка` (корневой `CMakeLists.txt`). В `CMakeSettings.json` для этой же сборки:

```json
{
  "configurations": [
    {
      "name": "x64-Release",
      "generator": "Visual Studio 17 2022",
      "configurationType": "Release",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "buildRoot": "${projectDir}\\build\\${name}",
      "installRoot": "${projectDir}\\install\\${name}",
      "cmakeCommandArgs": "-A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64",
      "buildCommandArgs": "",
      "ctestCommandArgs": "",
      "variables": []
    }
  ]
}
```

`VULKAN_SDK` задайте в окружении до открытия папки.

### Только консоль

`-DNOGUI=ON` на MSVC переключает runtime на статический (`/MT`) и просит статический Boost. Triplet vcpkg — `x64-windows-static`. Qt и Vulkan не используются.

```powershell
C:\vcpkg\vcpkg.exe install --triplet x64-windows-static `
  boost-filesystem boost-system boost-regex boost-random `
  boost-uuid boost-crc boost-algorithm zlib
```

```powershell
cd C:\path\to\tool1cd
mkdir build-console
cd build-console
cmake -G "Visual Studio 17 2022" -A x64 -DNOGUI=ON `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
  -DVCPKG_TARGET_TRIPLET=x64-windows-static `
  ..
cmake --build . --config Release --target ctool1cd --target testproject
```

`ctool1cd.exe` и `testproject.exe` лежат в `build-console\bin\Release\`.

## WebAssembly (браузерный просмотр таблиц)

Команды рассчитаны на bash: Linux, macOS, WSL или Git Bash. `em++` должен быть в `PATH` в той же сессии, где запускается сборка.

### Emscripten

```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

В новом терминале снова выполните `source /path/to/emsdk/emsdk_env.sh`.

### Boost для Emscripten

Скрипту `web/build-wasm.sh` нужны статические `filesystem`, `system` и `regex`. Заголовки `uuid`, `crc` и `algorithm` приезжают с исходниками Boost. Отдельный zlib не нужен: скрипт включает его флагом `-sUSE_ZLIB=1`.

```sh
curl -L -o boost_1_88_0.tar.gz https://archives.boost.org/release/1.88.0/source/boost_1_88_0.tar.gz
tar xf boost_1_88_0.tar.gz
cd boost_1_88_0

./bootstrap.sh
cat > user-config.jam <<'EOF'
using clang : emscripten : em++ ;
EOF

export BOOST_WASM_ROOT="$HOME/opt/boost-wasm"
mkdir -p "$BOOST_WASM_ROOT"

./b2 -j"$(nproc)" \
  toolset=clang-emscripten \
  target-os=emscripten \
  variant=release \
  link=static runtime-link=static \
  threading=single \
  --with-system --with-filesystem --with-regex \
  cxxflags="-O3" \
  install --prefix="$BOOST_WASM_ROOT"
```

`threading=single` оставляет имена библиотек без суффикса (`libboost_filesystem.a`), на них рассчитан скрипт.

### Сборка WASM

```sh
source /path/to/emsdk/emsdk_env.sh
BOOST_WASM_ROOT="$HOME/opt/boost-wasm" ./web/build-wasm.sh
```

После успешной сборки появятся:

- `web/parser.js`
- `web/parser.wasm`

### Локальный запуск

```sh
cd web
python3 -m http.server 8080
```

Открыть в браузере:

`http://localhost:8080`
