#include<iostream>
#include<cstring>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
using namespace std;
int main(int argc,char* argv[])
{
    if(argc != 2){
        cerr<< "usage: ./backend_server <port>" << endl;
        return 1;
    }
    int port = stoi(argv[1]);

    // create socket
    int server_fd = socket(AF_INET,SOCK_STREAM,0);
    if(server_fd == -1)
    {
        perror("socket failed");
        return 1;
    }

    // adress structure
    sockaddr_in server_addr{};
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    //bind the socket
    if(::bind(server_fd,(struct sockaddr*)&server_addr , sizeof(server_addr)) == -1)
    {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    //listen
    if(listen(server_fd,5) == -1) // 5 is the backlog pending connection system should queue
    {
        perror("listen failed");
        close(server_fd);
        return 1;
    } 
    cout << "Backend server listening on a port : " << port << endl;

    while(true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_len);
        if(client_fd == -1)
        {
            perror("client accept failed");
            return 1;
        }

        char buffer[1024];
        ssize_t data_recieved = recv(client_fd, buffer, sizeof(buffer) -1 ,0); // recv expects raw writable buffer so using char array
        if(data_recieved > 0)
        {
            buffer[data_recieved] = '\0';
            string response = "Echo from port: " + to_string(port)+ ":"+ buffer;
            send(client_fd,response.c_str(),response.length(),0);
        }
        close(client_fd);
    }
    close(server_fd);
    return 0;

}