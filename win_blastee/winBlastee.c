
/*****************************************************************************
 * blastee - server program for MS WINDOWS host
 *
 * DESCRIPTION
 *
 *     This is a server program which accepts a TCP connection from a single client.
 *     It allows to configure the maximum  size  of  the  socket-level
 *     receive buffer. It repeatedly reads from the TCP socket, displaying
 *     the amount of data received.
 *
 * EXAMPLE:
 *
 *     To run this blastee program from your WINDOWS host do the following at
 *     a DOS prompt: 
 *     C:\> blastee  7000  1000  16000
 *
 *
 */

#include <sys/types.h>
#include <winsock2.h>				/* <sys/socket.h> */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <io.h>
#include <ws2tcpip.h>


static int64_t total_bytes_received;


static void blast_receive (const int client_socket, const int size)
{
    char *const buffer = (char *) malloc(size);

	if (buffer == NULL)
    {
		perror ("cannot allocate buffer");
		exit (EXIT_FAILURE);
    }

    for (;;)
    {
		int numRead;

		if ((numRead = recv (client_socket, buffer, size, 0)) < 0) 
        {
		    perror ("blastee read error");
		    break;
		}
		if (numRead == 0 ){
		    break; /* disconnect by peer */
		}

		total_bytes_received += numRead;
    }
    free (buffer);
    closesocket (client_socket);
}


int main (int argc, char *argv[])
{
    WORD wVersionRequested; 
    WSADATA wsaData; 
    int err; 
    int port, size, blen;
    int listening_socket;
    int client_socket;
    int on = 1;
    socklen_t len;
    char client_ip_str[20];

    struct sockaddr_in	serverAddr; /* server's address */
    struct sockaddr_in  clientAddr; /* client's address */

    /* initialize winsock.dll */
    wVersionRequested = MAKEWORD(1, 1); 
    err = WSAStartup(wVersionRequested, &wsaData); 
 
    if (err != 0) 
    {
        /* Tell the user that we couldn't find a useable */ 
        /* winsock.dll.     */ 
        exit (EXIT_FAILURE);
    }
 
    /* Confirm that the Windows Sockets DLL supports 1.1.*/ 
    /* Note that if the DLL supports versions greater */ 
    /* than 1.1 in addition to 1.1, it will still return */ 
    /* 1.1 in wVersion since that is the version we */ 
    /* requested. */ 
 
    if ( LOBYTE( wsaData.wVersion ) != 1 || 
		    HIBYTE( wsaData.wVersion ) != 1 ) 
    { 
    	/* Tell the user that we couldn't find a useable */ 
	    /* winsock.dll. */ 
        perror ("Unable to initialize WinSock Version 1.1\n");
        WSACleanup(); 
        exit (EXIT_FAILURE);
	} 

    /* Process command line arguments */
    if (argc < 4)
	{
        printf ("usage: %s port size bufLen\n", argv [0]);
        exit (EXIT_FAILURE);
	}

    port = atoi (argv [1]);
    size = atoi (argv [2]);
    blen = atoi (argv [3]); /* SO_RCVBUF */

    memset (&serverAddr, 0, sizeof (serverAddr));
    memset (&clientAddr, 0, sizeof (clientAddr));

    /* Open the socket. Use ARPA Internet address format and stream sockets. */
    listening_socket = socket (AF_INET, SOCK_STREAM, 0);

    if (listening_socket < 0) {
		perror ("cannot open socket");
        exit (EXIT_FAILURE);
    }
    /* maximum size of socket-level receive buffer */
    if (setsockopt (listening_socket, SOL_SOCKET, SO_RCVBUF, (char *)&blen, sizeof (blen)) == -1) 
    {
		perror ("setsockopt SO_RCVBUF failed");
        exit (EXIT_FAILURE);
    }
    if (setsockopt (listening_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) == -1)
    {
		perror ("setsockopt SO_REUSEADDR failed");
        exit (EXIT_FAILURE);
    }

	/* Set up our internet address, and bind it so the client can connect. */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port	= htons (port);

    if (bind (listening_socket, (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) 
    {
    	perror ("bind error");
        exit (EXIT_FAILURE);
    }

    /* Listen, for the client to connect to us. */
    if (listen (listening_socket, 2) < 0) 
    {
    	perror ("listen failed");
        exit (EXIT_FAILURE);
    }

    len = sizeof (clientAddr);

    /* Accept the connection from the client */
    client_socket = accept (listening_socket, (struct sockaddr *) &clientAddr, &len);
    if (client_socket == -1) {
        perror ("accept failed");
        closesocket (listening_socket);
        exit (EXIT_FAILURE);
    }

    printf ("Accepted connection from client IP %s port %u\n", 
        inet_ntop (AF_INET, &clientAddr.sin_addr, client_ip_str, sizeof (client_ip_str)),
        ntohs (clientAddr.sin_port));

    /* Since only handle a single client, close the listening socket as don't accept any further connections */
    closesocket (listening_socket);

    blast_receive (client_socket, size);

    return EXIT_SUCCESS;
}
