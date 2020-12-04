#include <stdio.h>
#include <stdlib.h>
#include "TFTPServer.h"
int main()
{

    if(tftp_server_init() == TFTPSERV_OK)printf("Server has been succefully intialised!\n");
    tftp_server_run();
    return 0;
}
