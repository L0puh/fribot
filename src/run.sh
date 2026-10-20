#!/bin/bash

# usage: ./run.sh [clean|build|flash|monitor|all]

set -e

[ -e "$ESP_IDF_PATH" ] || { echo "failed to find $ESP_IDF_PATH,
                                  make sure you set \$ESP_IDF_PATH"; exit 1; }

[ -n "$IDF_PATH" ] || source $ESP_IDF_PATH/export.sh

PORT="/dev/ttyUSB0"
TARGET="esp32"  

CMD=$1
PACK=$2


if [ -z "$CMD" ]; then 
   echo "no cmd is provided"
   echo """
   clean {module}
   build {module}
   flash {module}
   monitor {module}
   all {module} (build -> flash -> monitor)
   release (builds all modules)
   """
   exit 1
fi

if [ -z "$PACK" ]; then 
   echo "no argument is provided, specify which module to $CMD"
   exit 1
elif [ ! -e "$PACK" ]; then
   echo "no such module, check the needed folder"
   exit 1
fi

echo "ESP PATH: $IDF_PATH" 
echo "PORT:     $PORT" 
echo "TARGET:   $TARGET" 


case "$1" in
    set-target) 
       echo  "setting target to $TARGET" 
       ( cd $PACK && idf.py set-target "$TARGET" 2>/dev/null )
       ;;

    clean)
        echo "cleaning build $PACK..."
        ( 
          cd $PACK
          rm -rf build sdkconfig
          idf.py reconfigure 
          idf.py set-target "$TARGET"
        )
        ;;

    build)
        echo "building $PACK..."
        ( 
          cd $PACK
          idf.py build 
        )
        ;;
    flash)
        echo "flashing $PACK to $PORT..."
        ( 
          cd $PACK
          idf.py -p "$PORT" flash
        )
        ;;
    monitor)
        echo "starting monitor $PACK on $PORT..."
        ( 
          cd $PACK
          idf.py -p "$PORT" monitor
        )
        ;;
    all)
        echo "$PACK: building -> flashing -> monitoring..."
        ( 
          cd $PACK
          idf.py -p "$PORT" build flash monitor
        )
        ;;
    *)
        echo "usage: ./run.sh [clean|target|build|flash|monitor|all] {module}"
        exit 1
        ;;
esac

