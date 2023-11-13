// webserver.c
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0); //Create tcp socket (Probably, the 0 argument makes it chose the most appropriate one, which is most likley tcp)
    if (sockfd == -1) {
        perror("error: bad server socket creation");
        return 1;
    }
    printf("server: server socket created successfully\n");

    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //create the host address

    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr); //create a client address

    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) { //bind server socket to address, this actually connects it
        perror("error: server socket cannot be bound to host address");
        return 1;
    }
    printf("server: server socket successfully bound to address\n");

    if (listen(sockfd, SOMAXCONN) != 0) { //listen to socket. The SOMAXCONN sets the max number of backlogs to the amount the system supports
        perror("error: server cannot begin listening");
        return 1;
    }
    printf("server: began listening for connections\n");

    for (;;) {
        // Accept incoming connections
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr,     //wait until it gets a response from client (this is blocking code)
                               (socklen_t *)&host_addrlen);
        if (newsockfd < 0) {
            perror("error: cannot accept connection");
            continue;
        }
        printf("server: packet accepted\n\n");

        int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr, //get the client address
                                (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            perror("error: cannot get client address");
            continue;
        }

        int valread = read(newsockfd, buffer, BUFFER_SIZE); //read the value from the socket
        if (valread < 0) {
            perror("error: cannot read request");
            continue;
        }

        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        //sscanf(buffer, "%s %s %s", method, uri, version);
        //printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port), method, version, uri);
        printf("RECIVED REQUEST:\n\n%s\n\n",buffer);
        
        char resp[] = "HTTP/1.0 200 OK\n"              //basic http response
          "Server: webserver-c\n"
          "Content-type: text/html\n\n"
          "<!DOCTYPE html>\n"
          "<html>\n"
          "  <head>\n"
          "   <link href=\"stylesheet.css\" rel=\"stylesheet\" type=\"text/css\" />\n"  // the client will put in a request for this
          "  </head>\n"
          "  <body>\n"
          "  <h1>Hello, World</h1>\n"
          "  <a href=\"fakeAddress.html\">test</a>\n"  //the client may put in a request when we send this, but it may also request it when the client clicks on the link
          "  <script>"
          " const xhr=new XMLHttpRequest;xhr.open(\"GET\",\"nonexistantfile.json\"),xhr.send(),xhr.responseType=\"json\",xhr.onload=()=>{if(4==xhr.readyState&&200==xhr.status){let e=xhr.response;console.log(e)}else console.log(`Error: ${xhr.status}`)}; "
          "  </script>"
          "  <body>\n"
          "</html>\n";

        int valwrite = write(newsockfd, resp, strlen(resp)); //write data to the socket (sending to client)
        if (valwrite < 0) {
            perror("error: writing data to client socket");
            continue;
        }

        close(newsockfd);
    }

    return 0;
}
