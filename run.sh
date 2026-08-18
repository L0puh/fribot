#!/bin/bash

# usage: ./run.sh [clean|build|flash|monitor|all]

set -e

[ -e "$ESP_IDF_PATH" ] || { echo "failed to find $ESP_IDF_PATH,
                                  make sure you set \$ESP_IDF_PATH"; exit 1; }

[ -n "$IDF_PATH" ] || source $ESP_IDF_PATH/export.sh

PORT="/dev/ttyUSB0"
TARGET="esp32"  

echo "ESP PATH: $IDF_PATH" 
echo "PORT:     $PORT" 
echo "TARGET:   $TARGET" 


case "$1" in
    set-target) 
       echo  "setting target to $TARGET" 
       idf.py set-target "$TARGET" 2>/dev/null 
       ;;

    clean)
        echo "cleaning build..."
        rm -rf build sdkconfig
        idf.py reconfigure
        idf.py set-target "$TARGET"
        ;;
    build)
        echo "building..."
        idf.py build
        ;;
    flash)
        echo "flashing to $PORT..."
        idf.py -p "$PORT" flash
        ;;
    monitor)
        echo "starting monitor on $PORT..."
        idf.py -p "$PORT" monitor
        ;;
    all)
        echo "building -> flashing -> monitoring..."
        idf.py -p "$PORT" build flash monitor
        ;;
    *)
        echo "usage: ./run.sh [clean|target|build|flash|monitor|all]"
        exit 1
        ;;
esac
