
/*
 * tftp_server.h
 *
 *  Created on: 29 oct. 2018
 *      Author: Vincent Van Assche
 */

#ifndef TFTP_SERVER_H_
#define TFTP_SERVER_H_

#define MAX_CHUNCK_SIZE 512	// Maximal number of octets from the file sent in one packet

#include <winsock2.h>
#include "stdio.h"

/**
 * Error codes to be used by TFTP�functions
 */
enum tftp_err {
	TFTPSERV_UNDEFINED_ERROR = 0,
	TFTPSERV_NO_SUCH_FILE = 1,
	TFTPSERV_ACCESS_VIOLATION = 2,
	TFTPSERV_DISK_FULL = 3,
	TFTPSERV_ILLEGAL_OPERATION = 4,
	TFTPSERV_UNKNOWN_ID = 5,
	TFTPSERV_FILE_ALLREADY_EXISTS = 6,
	TFTPSERV_NO_SUCH_USER = 7,
	TFTPSERV_OK,
	TFTPSERV_ERR,
};

/***
 * Initialize the file system by calling � tftp_file_init() � and creates and bind the netconn
 */
enum tftp_err tftp_server_init(void);

/***
 * Wait for a request and handle the file transfer
 */
void tftp_server_run(void );

#endif /* TFTP_SERVER_H_ */
