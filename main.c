#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>
#include "expect.h"

#define CHUNK_SIZE 2048

int irc_send(int socket, char* fmt,...) {
  char msg[512];
  va_list args;

  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);

  puts(msg);
  return send(socket, msg, strlen(msg), 0);
}

char* irc_get(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

int main() {
  int socket_desc;
  struct sockaddr_in server;
  char* recv_chunk[CHUNK_SIZE];
  int ret;

  char *hostname = "irc.freenode.net";
  char ip[100];
  struct hostent *he;
  struct in_addr **addr_list;

  he = gethostbyname(hostname);
  expect(he != NULL, "Error getting host name");
  addr_list = (struct in_addr **)he->h_addr_list;
  for (int i = 0; addr_list[i] != NULL; i++) {
    strcpy(ip, inet_ntoa(*addr_list[i]));
  }

  printf("Connecting to %s...\n", ip);

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  expect(socket_desc != -1, "Could not create socket!");
  printf("Socket client created!\n");

  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(6667);
  
  ret = connect(socket_desc, (struct sockaddr *)&server, sizeof(server));
  expect(ret >= 0, "Connection error!");
  printf("Connected to server!\n");

  ret = 1;

  while (ret > 0) {
    ret = recv(socket_desc, recv_chunk, CHUNK_SIZE, 0);
    expect(ret >= 0, "Recv failed!");

    char* server_response = (char*) recv_chunk;
    puts(server_response);

    if (strstr(server_response, "Found your hostname") != NULL) {
      irc_send(socket_desc, "NICK %s\r\n", "huytestnick1");
      irc_send(socket_desc, "USER %s 0 * :%s\r\n", "huytestnick1", "Huy Tran");
      irc_send(socket_desc, "JOIN %s\r\n", "#huytestroom");
    }
  }

  close(socket_desc);

  return 0;
}
