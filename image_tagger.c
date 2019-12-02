#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// constants
static char const * const HTTP_200_FORMAT = "HTTP/1.1 200 OK\r\n\
Content-Type: text/html\r\n\
Content-Length: %ld\r\n\r\n";
static char const * const HTTP_400 = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_400_LENGTH = 47;
static char const * const HTTP_404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
static int const HTTP_404_LENGTH = 45;

int player1;
int player2;

bool player1_ready;
bool player2_ready;

char player1_k[2005][205];
char player2_k[2005][205];

int player1_knum;
int player2_knum;

bool isquit;
bool isgameover;

// represents the types of method
typedef enum
{
    GET,
    POST,
    UNKNOWN
} METHOD;

static bool handle_http_request(int sockfd)
{
    // try to read the request
    char buff[2049];
    int n = read(sockfd, buff, 2049);
    //fwrite(buff,1,2049,stdout);
    if (n <= 0)
    {
        if (n < 0)
            perror("read");
        else
            printf("socket %d close the connection\n", sockfd);
        return false;
    }
    if( player1 <0 && player2 != sockfd){
      player1 = sockfd;
    }
    if( player2 <0 && player1 != sockfd){
      player2 = sockfd;
    }

    // terminate the string
    buff[n] = 0;

    //fwrite(buff,1,2049,stdout);

    char * curr = buff;

    // parse the method
    METHOD method = UNKNOWN;
    if (strncmp(curr, "GET ", 4) == 0)
    {
        curr += 4;
        method = GET;
    }
    else if (strncmp(curr, "POST ", 5) == 0)
    {
        curr += 5;
        method = POST;
    }
    else if (write(sockfd, HTTP_400, HTTP_400_LENGTH) < 0)
    {
        perror("write");
        return false;
    }

    // sanitise the URI
    while (*curr == '.' || *curr == '/')
        ++curr;
    // assume the only valid request URI is "/" but it can be modified to accept more files



if (strncmp(curr, "?start", 6) == 0){
      if (method == GET)
      {   if(player1 == sockfd){
        player1_ready = true;
        }
        if(player2 == sockfd){
          player2_ready = true;
        }

          // get the size of the file
          struct stat st;
          stat("3_first_turn.html", &st);
          n = sprintf(buff, HTTP_200_FORMAT, st.st_size);

          // send the header first
          if (write(sockfd, buff, n) < 0)
          {
              perror("write");
              return false;
          }
          // send the file
          int filefd = open("3_first_turn.html", O_RDONLY);
          do
          {
              n = sendfile(sockfd, filefd, NULL, 2048);
          }
          while (n > 0);
          if (n < 0)
          {
              perror("sendfile");
              close(filefd);
              return false;
          }
          close(filefd);
      }
      else if(method == POST){
        char * quit = strstr(buff, "quit=");
        char * guess = strstr(buff, "guess=");
        if (quit) {
          isquit = true;
          player1_ready = false;
          player2_ready = false;

          player1 = -1;
          player2 = -1;

          player1_knum = 0;
          player2_knum = 0;
          // get the size of the file
          struct stat st;
          stat("7_gameover.html", &st);
          n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
          // send the header first
          if (write(sockfd, buff, n) < 0)
          {
              perror("write");
              return false;
          }
          // send the file
          int filefd = open("7_gameover.html", O_RDONLY);
          do
          {
              n = sendfile(sockfd, filefd, NULL, 2048);
          }
          while (n > 0);
          if (n < 0)
          {
              perror("sendfile");
              close(filefd);
              return false;
          }
          close(filefd);
        }
        if(guess){
          // get the size of the file
            if(player1_ready&&player2_ready){
              char * keyword = strstr(buff, "keyword=") + 8;
              int keyword_length = strlen(keyword);
              if(player1 == sockfd){
                strncpy(player1_k[player1_knum], keyword, keyword_length);
                player1_knum += 1;
                int flag =0;
                for(int i=0; i<=player2_knum;i++){
                  if(strcmp(keyword,player2_k[i])==0){
                    flag = 1;
                  }
                }
                if(flag){
                    struct stat st;
                    stat("6_endgame.html", &st);
                    n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
                    // send the header first
                    if (write(sockfd, buff, n) < 0)
                    {
                        perror("write");
                        return false;
                    }
                    // send the file
                    int filefd = open("6_endgame.html", O_RDONLY);
                    do
                    {
                        n = sendfile(sockfd, filefd, NULL, 2048);
                    }
                    while (n > 0);
                    if (n < 0)
                    {
                        perror("sendfile");
                        close(filefd);
                        return false;
                    }
                    close(filefd);
                }
                else{
                  struct stat st;
                  stat("4_accepted.html", &st);
                  n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
                  // send the header first
                  if (write(sockfd, buff, n) < 0)
                  {
                      perror("write");
                      return false;
                  }
                  // send the file
                  int filefd = open("4_accepted.html", O_RDONLY);
                  do
                  {
                      n = sendfile(sockfd, filefd, NULL, 2048);
                  }
                  while (n > 0);
                  if (n < 0)
                  {
                      perror("sendfile");
                      close(filefd);
                      return false;
                  }
                  close(filefd);
                }
              }

              if(player2 == sockfd){
                strncpy(player2_k[player2_knum], keyword, keyword_length);
                player2_knum += 1;
                int flag =0;
                for(int i=0; i<=player1_knum;i++){
                  if(strcmp(keyword,player1_k[i])==0){
                    flag = 1;
                  }
                }
                if(flag){
                    isgameover = true;
                    struct stat st;
                    stat("6_endgame.html", &st);
                    n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
                    // send the header first
                    if (write(sockfd, buff, n) < 0)
                    {
                        perror("write");
                        return false;
                    }
                    // send the file
                    int filefd = open("6_endgame.html", O_RDONLY);
                    do
                    {
                        n = sendfile(sockfd, filefd, NULL, 2048);
                    }
                    while (n > 0);
                    if (n < 0)
                    {
                        perror("sendfile");
                        close(filefd);
                        return false;
                    }
                    close(filefd);
                }
                else{
                  struct stat st;
                  stat("4_accepted.html", &st);
                  n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
                  // send the header first
                  if (write(sockfd, buff, n) < 0)
                  {
                      perror("write");
                      return false;
                  }
                  // send the file
                  int filefd = open("4_accepted.html", O_RDONLY);
                  do
                  {
                      n = sendfile(sockfd, filefd, NULL, 2048);
                  }
                  while (n > 0);
                  if (n < 0)
                  {
                      perror("sendfile");
                      close(filefd);
                      return false;
                  }
                  close(filefd);
                }
              }

        }
            else if(isquit = true){

                  struct stat st;
                  stat("6_endgame.html", &st);
                  n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
                  // send the header first
                  if (write(sockfd, buff, n) < 0)
                  {
                      perror("write");
                      return false;
                  }
                  // send the file
                  int filefd = open("6_endgame.html", O_RDONLY);
                  do
                  {
                      n = sendfile(sockfd, filefd, NULL, 2048);
                  }
                  while (n > 0);
                  if (n < 0)
                  {
                      perror("sendfile");
                      close(filefd);
                      return false;
                  }
                  close(filefd);
            }
            else{
              struct stat st;
              stat("5_discarded.html", &st);
              n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
              // send the header first
              if (write(sockfd, buff, n) < 0)
              {
                  perror("write");
                  return false;
              }
              // send the file
              int filefd = open("5_discarded.html", O_RDONLY);
              do
              {
                  n = sendfile(sockfd, filefd, NULL, 2048);
              }
              while (n > 0);
              if (n < 0)
              {
                  perror("sendfile");
                  close(filefd);
                  return false;
              }
              close(filefd);
            }
        }

      }
    }



if (*curr == ' '){

        if (method == GET){
            // get the size of the file
            struct stat st;
            stat("1_intro.html", &st);
            n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
            // send the header first
            if (write(sockfd, buff, n) < 0)
            {
                perror("write");
                return false;
            }
            // send the file
            int filefd = open("1_intro.html", O_RDONLY);
            do
            {
                n = sendfile(sockfd, filefd, NULL, 2048);
            }
            while (n > 0);
            if (n < 0)
            {
                perror("sendfile");
                close(filefd);
                return false;
            }
            close(filefd);
        }

        else if (method == POST)
        {
            // locate the username, it is safe to do so in this sample code, but usually the result is expected to be
            // copied to another buffer using strcpy or strncpy to ensure that it will not be overwritten.

            char * quit = strstr(buff, "quit=");
            if (quit) {
              isquit = true;
              player1_ready = false;
              player2_ready = false;

              player1 = -1;
              player2 = -1;

              player1_knum = 0;
              player2_knum = 0;
              // get the size of the file
              struct stat st;
              stat("7_gameover.html", &st);
              n = sprintf(buff, HTTP_200_FORMAT, st.st_size);
              // send the header first
              if (write(sockfd, buff, n) < 0)
              {
                  perror("write");
                  return false;
              }
              // send the file
              int filefd = open("7_gameover.html", O_RDONLY);
              do
              {
                  n = sendfile(sockfd, filefd, NULL, 2048);
              }
              while (n > 0);
              if (n < 0)
              {
                  perror("sendfile");
                  close(filefd);
                  return false;
              }
              close(filefd);
            }
            else{
              char * username = strstr(buff, "user=") + 5;
              int username_length = strlen(username);
              char userbuff[2049];
              strncpy(userbuff, username, username_length);
              userbuff[username_length]=0;

            // get the size of the file
              struct stat st;
              stat("2_start.html", &st);
              // increase file size to accommodate the username
              long size = st.st_size + username_length;
              n = sprintf(buff, HTTP_200_FORMAT, size);
              // send the header first
              if (write(sockfd, buff, n) < 0)
              {
                  perror("write");
                  return false;
              }
              // read the content of the HTML file
              int filefd = open("2_start.html", O_RDONLY);
              n = read(filefd, buff, 2048);
              if (n < 0)
              {
                  perror("read");
                  close(filefd);
                  return false;
              }
              close(filefd);
              //move the trailing part backward
              int p1, p2;
              for (p1 = size - 1, p2 = p1 - username_length; p1 >= size - 212; --p1, --p2)
                  buff[p1] = buff[p2];

              ++p2;

              // copy the username

              for(int i=0;i<username_length;i++){
                buff[p2]=userbuff[i];
                p2++;
              }


            if (write(sockfd, buff, size) < 0)
            {
                perror("write");
                return false;
            }

          }
        }

        else{// never used, just for completeness
            fprintf(stderr, "no other methods supported");
            // send 404
            if (write(sockfd, HTTP_404, HTTP_404_LENGTH) < 0)
            {
              perror("write");
              return false;
            }
          }
    }
    return true;
}

int main(int argc, char * argv[])
{
     player1_ready = false;
     player2_ready = false;

     isquit = false;
     isgameover = false;

     player1 = -1;
     player2 = -1;

     player1_knum = 0;
     player2_knum = 0;

    if (argc < 3)
    {
        fprintf(stderr, "usage: %s ip port\n", argv[0]);
        return 0;
    }
    else if(argc == 3) {
  fprintf(stderr, "image_tagger server is now running at IP: %s on port %s\n",
    argv[1], argv[2]);}

    // create TCP socket which only accept IPv4
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // reuse the socket if possible
    int const reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // create and initialise address we will listen on
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // if ip parameter is not specified
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // bind address to socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // listen on the socket
    listen(sockfd, 5);

    // initialise an active file descriptors set
    fd_set masterfds;
    FD_ZERO(&masterfds);
    FD_SET(sockfd, &masterfds);
    // record the maximum socket number
    int maxfd = sockfd;

    while (1)
    {
        // monitor file descriptors
        fd_set readfds = masterfds;
        if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // loop all possible descriptor
        for (int i = 0; i <= maxfd; ++i){
            // determine if the current file descriptor is active
            if (FD_ISSET(i, &readfds))
            {
                // create new socket if there is new incoming connection request
                if (i == sockfd)
                {
                    struct sockaddr_in cliaddr;
                    socklen_t clilen = sizeof(cliaddr);
                    int newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
                    if (newsockfd < 0)
                        perror("accept");
                    else
                    {
                        // add the socket to the set
                        FD_SET(newsockfd, &masterfds);
                        // update the maximum tracker
                        if (newsockfd > maxfd)
                            maxfd = newsockfd;
                        // print out the IP and the socket number
                        char ip[INET_ADDRSTRLEN];
                        printf(
                            "new connection from %s on socket %d\n",
                            // convert to human readable string
                            inet_ntop(cliaddr.sin_family, &cliaddr.sin_addr, ip, INET_ADDRSTRLEN),
                            newsockfd
                        );
                    }

                }

                // a request is sent from the client
                else if (!handle_http_request(i))

                {
                    close(i);
                    FD_CLR(i, &masterfds);
                }

             }
          }

    }

    return 0;
}
