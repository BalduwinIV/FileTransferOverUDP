usage: 
    
    data_sender [--help] [--ip=LOCAL_IP] [--port=LOCAL_PORT] [--dest_ip=TARGET_IP] [--dest_port=TARGET_PORT] [--file=FILENAME] [listen/send/stop]

options:

    --help                      Show this help message and exit.
    --ip=LOCAL_IP               Sets your ip address to bind it with LOCAL_PORT.
    --port=LOCAL_PORT           Defines the port that will be open for data transfering.
    --dest_ip=TARGET_IP         Sets the ip adress of a user, who will receive data.
    --dest_port=TARGET_PORT     Defines the port of a user, who will receive data.
    --CRC=CRC_CODE              Defines CRC code by number (1101 for CRC-3, for example).
    --n=PACKETS_BUFFER_SIZE     Defines size of packets buffer.
    --filename=FILENAME         Defines the name of file, that yo want to send.
commands:
    
    data_sender --ip=LOCAL_IP --port=LOCAL_PORT listen      Start listening.
    data_sender stop                                        Stop listening.
    data_sender --ip=LOCAL_IP --port=LOCAL_PORT --dest_ip=TARGET_IP --dest_port=TARGET_PORT --filename=FILENAME send        Send file \"FILENAME\" to TARGET_IP:PORT address.
