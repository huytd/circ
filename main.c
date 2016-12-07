#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <poll.h>
#include "expect.h"

#define CHUNK_SIZE 2048
#define RL_BUFSIZE 1024

#define clear() printf("\033[H\033[J")
#define gotoxy(x,y) printf("\033[%d;%dH", (x), (y))

char* term_readline() {
  int position;
  int bufsize;
  char* buffer;
  char c;

  position = 0;
  bufsize = sizeof(char) * RL_BUFSIZE;
  buffer = malloc(bufsize);
  if (!buffer) {
    fprintf(stderr, "ERROR: Could not allocate buffer!\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }

    if (position > bufsize) {
      return buffer;
    }
    
    position++;
  }
}

int term_execute(const char* line) {
  if (strcmp(line, "exit") == 0) {
    return 1;
  }
  return 0;
}

int irc_send(int socket, char* fmt,...) {
  char msg[512];
  va_list args;

  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  va_end(args);

  puts(msg);
  return send(socket, msg, strlen(msg), 0);
}

int main() {
  int max_x, max_y;

  int socket_desc;
  struct sockaddr_in server;
  int ret;

  char *hostname = "irc.freenode.net";
  char ip[100];
  struct hostent *he;
  struct in_addr **addr_list;

  initscr();
  noecho();
  curs_set(FALSE);

  getmaxyx(stdscr, max_y, max_x);

  he = gethostbyname(hostname);
  expect(he != NULL, "Error getting host name");
  addr_list = (struct in_addr **)he->h_addr_list;
  for (int i = 0; addr_list[i] != NULL; i++) {
    strcpy(ip, inet_ntoa(*addr_list[i]));
  }

  //printf("Connecting to %s...\n", ip);

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  expect(socket_desc != -1, "Could not create socket!");
  //printf("Socket client created!\n");

  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(6667);
  
  ret = connect(socket_desc, (struct sockaddr *)&server, sizeof(server));
  expect(ret >= 0, "Connection error!");
  //printf("Connected to server!\n");

  ret = 1;

  int exit;
  char *line;
  
  struct pollfd fds[2];

  fds[0].fd = socket_desc;
  fds[0].events = POLLIN;

  fds[1].fd = fileno(stdin);
  fds[1].events = POLLIN;
  
  do {
    while( poll(fds, 2, 0) >= 0 ) {
      if (fds[0].revents & POLLIN) {
        char* recv_chunk;
        recv_chunk = malloc(sizeof(char) * CHUNK_SIZE);
        ret = recv(socket_desc, recv_chunk, CHUNK_SIZE, 0);

        char* server_response = recv_chunk;
        if (strlen(server_response) > 0) {
          printf("[%lu] %s\n", strlen(server_response), server_response);
        }
      } else if (fds[1].revents & POLLIN) {
        printf("> ");

        line = term_readline();
        exit = term_execute(line);

        if (exit == 0) {
          if (strcmp(line, "login") == 0) {
            irc_send(socket_desc, "NICK %s\r\n", "huytestnick1");
            irc_send(socket_desc, "USER %s 0 * :%s\r\n", "huytestnick1", "Huy Tran");
            irc_send(socket_desc, "JOIN %s\r\n", "#huytestroom");
          } else if (strcmp(line, "") != 0) {
            irc_send(socket_desc, "%s\r\n", line);
          }
        }

        free(line);
      }
    }
  } while(!exit);

  close(socket_desc);

  return 0;
}
