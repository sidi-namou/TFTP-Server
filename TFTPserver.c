#include "TFTPServer.h"
#define TFTP_PORT   69
#include "winsock2.h"
#include <stdlib.h>
#include <stdint-gcc.h>



/*******/

#define COUNT (MAX_CHUNCK_SIZE -4)
#define ErrorFileSize 0
#define sendOK    1
#define ErrorSend     2
#define FileNameError  10

#define RQR                 1
#define WRQ                 2
#define DATA                3
#define ACK                 4
#define ERROR_RESPONSE      5

static int blockNumber = 1;

static int serv;         /** Identifiant du socket serveur*/

static int iResult;      /** Résultat des fonctions de la bibliothèque*/

static char package[MAX_CHUNCK_SIZE];   // Tampon pour stocker les données reçues

static char packageSend[MAX_CHUNCK_SIZE];   // Tampon pour stocker les données a envoyé

static int len;          // Taille de la zone mémoire disponible pour l'adresse du client

static int bytesRead;    // Longueur des données contenues dans le datagramme reçu

static int bytesSend;   // Longueur des données contenues dans le datagramme envoyé


static char FileName[100];
struct sockaddr_in servAddr;
struct sockaddr_storage clientAddr;

static int waitForRQ(void);
static int rqIsValid(void);
static int findSize(char file_name[]);
static int sendResponse(char file_name[]);
static int waitForACK();

static int sizeofFile;
static int sendError(uint16_t errorCode, char * errorMess);


/** This function is used to initialize the server **/

enum tftp_err tftp_server_init(void){


    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult !=0) {
        printf("WSAStartup failed: %d\n",iResult);
        return 1;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(TFTP_PORT);
    serv = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);

    if(serv == INVALID_SOCKET){

                    iResult = WSAGetLastError();
             printf("INVALID SOCKET %d\n",iResult);
             return  TFTPSERV_ERR;
    }

    iResult = bind(serv,(struct sockaddr *)&servAddr, sizeof(servAddr));
    if (iResult != 0) {

            printf("bind failed: %d\n",iResult);
            return TFTPSERV_ERR;
    }

        return TFTPSERV_OK;
}



/** This method is used to run the server **/

void tftp_server_run(void ){

    while(1){

            /** Initialize block number for each client */

                blockNumber=1;

                if((waitForRQ()!=0) && (rqIsValid()!=0)){

                            printf("TFTP SERVER IS RUNNING \n");

                            if(sendResponse(FileName)!=sendOK)

                                        printf("\n Server Cannot send data \n");

                            }

                else  {
                            printf("TFTP SERVER HAS ENCOUNTERED A PROBLEM \n");

                        }


            }




}

/** This method used to wait for ACK packet from the client */
/** Return type of this function is :
 0 when we get a socket ERROR or the package does not correspond to ACK package
 1 when we get an ACK from client */

static int waitForACK(){

    len = sizeof(struct sockaddr_in);

    ZeroMemory(&package,sizeof(package));

    printf("Waiting for ACK...\n");

    bytesRead = recvfrom(serv,package,MAX_CHUNCK_SIZE, 0, (struct sockaddr *) &clientAddr, &len);

     if (bytesRead == SOCKET_ERROR)
    {
        iResult = WSAGetLastError();
        printf("waitFORRQ Error: %d\n",iResult);
        return 0;
    }

    if(rqIsValid() != ACK )return 0;


    return 1;
}

/** This function is used to wait for client request **/
/** Return type of this function is :
 0 when we get a socket ERROR
 1 when we get a RQ from client */

static int waitForRQ(void){


    len = sizeof(struct sockaddr_in);

    ZeroMemory(&package,sizeof(package));

    printf("Waiting for datagram...\n");

    bytesRead = recvfrom(serv,package,MAX_CHUNCK_SIZE, 0, (struct sockaddr *) &clientAddr, &len);

    if (bytesRead == SOCKET_ERROR)
    {
        iResult = WSAGetLastError();
        printf("recvfrom Error: %d\n",iResult);
        return 0;
    }

    return 1;
}

/**
This function is used to analyze TFTP package
it returns :
0 when the opCode is not mentioned in TFTP specification
OpCode when OpCode is mentioned in TFTP specification

**/
static int rqIsValid(void){


                int opCode;                                         /** Operation Code */
                char mode[50];                                      /** Mode of TFTP */
                int block;                                          /** Block number */

                int i =2;                                           /** Index for package */
                int firstModeIndex;                                 /** Index to point to the first mode letter */


                opCode= package[0]*256 + package[1];                /** Convert opCode from 256 base to 10 base */


                /** Only Read Request, ACK, will be treated For this exercise*/



                switch(opCode){


                            case RQR:{

                                            /** Only octet mode will be allowed*/

                                            /** package will be opCode(2 Bytes)_FileName_0(1 Byte)_Mode_0(1 Byte)   */


                                            while(package[i] != 0 ){

                                            FileName[i - 2] = package[i];               /** Get Name file from package */
                                            i++;
                                            }

                                             FileName[i - 2] = '\0';                     /** Convert FileName to string */
                                             i++;
                                             firstModeIndex = i;


                                             while(package[i] != 0 ){

                                            mode[i - firstModeIndex] = package[i];          /** Get Mode from package */
                                             i++;
                                            }

                                            mode[i-firstModeIndex] = '\0';                  /** Convert mode to string */

                                            if(strcmp(mode,"octet") != 0){       /** If mode is not octet --> send error back to the client */

                                                if(sendError(0,"Only octet mode is supported by this implementation ")!=sendOK)printf("mode Error couldn't be send");
                                                return 0;

                                                    }

                                            /** Otherwise request is valid, So we will treat it as below */

                                            printf("\nMode of packet is : %s\n",mode);
                                            printf("\nName of file is : %s\n",FileName);

                                           /** print size of file*/
                                            sizeofFile = findSize(FileName);

                                            if(sizeofFile == -1)return FileNameError;

                                            printf("size of file is %d octets \n",sizeofFile);


                                            return RQR;

                                            break;
                                                    }

                        case WRQ:
                                    {

                                            if( sendError(0,"WRQ not supported by this implementation ") != sendOK )

                                                                printf("WRQ Error couldn't be send");

                                            return WRQ;

                                            break;

                                            }

                          case DATA:
                                       {
                                            if(sendError(0,"DATA not supported by this implementation ")!=sendOK)

                                            printf("DATA Error couldn't be send");

                                            return DATA;

                                            break;
                                }

                          case ACK:
                                    {
                                            block = package[2]*256 + package[3];         /** get the block number of ACK packet*/

                                           /**IF block does not correspond to block number send, Print ERROR to user*/
                                            if(block != blockNumber){

                                            printf(" Couldn't send ACK Error \n");

                                               return 0;

                                                    }
                                            /** Otherwise print data received to user */

                                            printf("ACK Received for block %d \n", block);

                                            return ACK;

                                            break;
                                        }

                         case ERROR_RESPONSE:
                                                {
                                                    if(sendError(0,"Error Client ")!=sendOK)
                                                        printf(" Couldn't send Client Error \n");

                                                    return ERROR_RESPONSE;

                                                    break;
                                                }
                        default:
                                    {
                                                /** operation Code does not expected */
                                                /** Send Error back to client with error code of 4*/

                                            if(sendError(4," Error, Code operation doesn't expected ")!=sendOK)

                                                printf(" Couldn't send Code Operation Error to client \n");

                                            return 0;

                                            break;
                                        }
                }


}

/**
This function is used to calculate a size of file

file_name: is a string that contain the name of the file

it returns :
-1 when the file does not exist
size of file when everything is okay.
**/

static int findSize(char file_name[]){

                // opening the file in read mode
                FILE* fp = fopen(file_name, "r");

                // checking if the file exist or not
                if (fp == NULL) {
                        if(sendError(1,"Error File Couldn't find in this database")!=sendOK)printf(" Couldn't send file Error \n");
                    printf("File Not Found!\n");
                    return -1;
                }

                fseek(fp, 0L, SEEK_END);

                // calculating the size of the file
                 int res = ftell(fp);

                // closing the file
                fclose(fp);

                return res;
}
/**
This function is used to send a file to client
it has one argument :
    -- File Name
@@@return type :
- ErrorSend : when an error has occurred
- sendOK : when the file is send.

**/

static int sendResponse(char file_name[]){


    FILE * file = fopen(file_name,"r");


   do{
        /** DATA Operation code is 0x0003 */
    packageSend[0] = 0;
    packageSend[1] = 3;     /** Data */

    /** Block number which occupied **/

    /**  block number = 256*x + y
    packageSend[2] = x; first Byte.
    packageSend[3] = y; second Byte.
    */

    packageSend[2] = blockNumber / 256;
    packageSend[3] = blockNumber % 256;

    /**Read correspond file and store data in packageSend */

    bytesSend = fread( packageSend + 4, 1, MAX_CHUNCK_SIZE, file );

    /** Print to user length of read successfully bytes for debugging **/

    printf("La taille des octets lus %d\n",bytesSend);


    /** Send response to client */

    if(bytesSend !=0){
                        /** Send data to client **/

            bytesSend = sendto(serv,packageSend,bytesSend+4,0,(struct sockaddr *) &clientAddr, len);

            if (bytesSend == SOCKET_ERROR)
            {
                        iResult = WSAGetLastError();
                        printf("Error: %d\n",iResult);
                        return ErrorSend;
            }

            printf("Number of bytes send is %d \n",bytesSend);

            /** Wait for ACK packet to send the next block of data, in case the file is bigger than 512 Bytes. */

            if(waitForACK() != 1)   return ErrorSend;   /** If the client does not respond correctly return an error **/

            blockNumber++;

            }

    }while(bytesSend!=0);   /**We will repeat the same process while byteSend is not equal to zero **/

    /** Once we send all content of file we close the file and return sendOK **/

    fclose(file);


    return sendOK;
}


/*** This function is used an error to the client and print this error in console to user **/

/**  errorCode is the error code in TFTP specification
     errorMess is the error message to send to the client
    **/

static int sendError(uint16_t errorCode, char * errorMess){

    /** We store the length of error message in a local variable **/

    int length = strlen(errorMess);

    /*** We specify the operation code in the head of the package **/
    packageSend[0] = 0;
    packageSend[1] = ERROR_RESPONSE;

    /** Set the error code in the package **/

    packageSend[2] = (uint8_t)(errorCode >> 8);
    packageSend[3] = (uint8_t)errorCode;

    for(int i=0; i < length; i++){

            packageSend[i + 4] = errorMess[i];      /** Set the error message in tail of the package */
    }

    packageSend[length + 4] = 0;                    /** Last Byte is 0 as described in TFTP specification */


    /** Send error to the client  **/

    bytesSend = sendto(serv,packageSend,length+5,0,(struct sockaddr *) &clientAddr, len);


    if (bytesSend == SOCKET_ERROR)
    {
            iResult = WSAGetLastError();
            printf("Error: %d\n",iResult);
            return ErrorSend;
    }

    printf("number of bytes send : %d \n",bytesSend);
    printf("Error message is : %s \n",packageSend+4);


return sendOK;

}

