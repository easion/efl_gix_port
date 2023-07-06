EFL
===

# 获取当前目录
current_directory=$(pwd)
current_time=$(date +%s)
find "$current_directory" -type f -mtime -2 -exec sudo rm {} \;


typedef Eina_Slice slice;

rm -f meson_options.txt; ln -s x11_options.txt meson_options.txt
rm -fr build; meson build --prefix=/usr/
clear; ninja -C build -v
clear; sudo ninja -C build install -v


export PATH=$PATH:/mnt/usb/orangepi5/efl-1.26.3/build/src/bin/eolian/
. /mnt/d1/project/linux_glibc_rk3588.sh
rm -f meson_options.txt; ln -s gix_options.txt meson_options.txt
rm -fr arm64-build; meson arm64-build --cross-file=cross_arm64.txt --prefix=/tmp/eflbin/
clear; ninja -C arm64-build -v


rm -f ./efl-1.26.3/meson_options.txt
rm -fr ./efl-1.26.3/build
rm -fr ./efl-1.26.3/arm64-build
tar Jcf efl-1.26.3.tar.xz ./efl-1.26.3/;sha256sum efl-1.26.3.tar.xz; cp -a efl-1.26.3.tar.xz /mnt/usb/orangepi5/openwrt/dl/



 ../src/lib/evas/gesture/efl_canvas_gesture_recognizer_triple_tap.eo
./build/src/lib/evas/gesture/efl_canvas_gesture_types.eot.h


strace /mnt/usb/orangepi5/efl-1.26.3/build/src/bin/eolian/eolian_gen -S -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/evas/gesture -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/eo -I /mnt/usb/orangepi5/efl-1.26.3/src/lib -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/efl/interfaces -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/ecore -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/eldbus -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/ecore_con -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/ector -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/evas -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/evas/include -o h:/mnt/usb/orangepi5/efl-1.26.3/arm64-build/src/lib/evas/gesture/efl_canvas_gesture_long_press.eo.h -o c:/mnt/usb/orangepi5/efl-1.26.3/arm64-build/src/lib/evas/gesture/efl_canvas_gesture_long_press.eo.c -o d:/mnt/usb/orangepi5/efl-1.26.3/arm64-build/src/lib/evas/gesture/efl_canvas_gesture_long_press.eo.d -e EVAS_API -gchd ../src/lib/evas/gesture/efl_canvas_gesture_long_press.eo

strace /usr/local/bin/eolian_gen -S -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/evas/gesture -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/eo -I /mnt/usb/orangepi5/efl-1.26.3/src/lib -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/efl/interfaces -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/ecore -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/eldbus -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/ecore_con -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/ector -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/evas -I /mnt/usb/orangepi5/efl-1.26.3/src/lib/evas/include -o h:/mnt/usb/orangepi5/efl-1.26.3/arm64-build/src/lib/evas/gesture/efl_canvas_gesture_long_press.eo.h -o c:/mnt/usb/orangepi5/efl-1.26.3/arm64-build/src/lib/evas/gesture/efl_canvas_gesture_long_press.eo.c -o d:/mnt/usb/orangepi5/efl-1.26.3/arm64-build/src/lib/evas/gesture/efl_canvas_gesture_long_press.eo.d -e EVAS_API -gchd ../src/lib/evas/gesture/efl_canvas_gesture_long_press.eo
