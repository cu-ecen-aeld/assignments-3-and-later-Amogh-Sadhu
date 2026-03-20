#!/bin/sh

    case $1 in
        start)
	    echo "Loading aesdchar driver"
      	    # Load the driver first to create /dev/aesdchar
	    aesdchar_load

            echo "Starting aesdsocket"
            start-stop-daemon -S -n aesdsocket  -a /usr/bin/aesdsocket -- -d
            ;;

        stop)
            echo "Stopping aesdsocket"
            start-stop-daemon -K -n aesdsocket
	    
	    echo "Unloading aesdchar driver"
            # Unload the driver to clean up the kernel module and /dev node
            aesdchar_unload
            ;;
        *)
            echo "Usage : $0 {start | stop}"
            exit 1
            ;;
    esac

exit 0
