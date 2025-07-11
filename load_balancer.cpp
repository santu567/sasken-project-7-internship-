#include<iostream>
#include<unistd.h>
#include<vector>
#include<netinet/in.h>
#include<cstring>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<thread>
using namespace std;


//backend servers
vector<pair<string,int>>backend_servers = {
    {"127.0.0.1", 9001},
    {"127.0.0.1", 9002},
    {"127.0.0.1", 9003}
};
vector<bool>backend_health(backend_servers.size(),true);
vector<int>request_count(backend_servers.size(),0);


int current_backend = 0;
mutex backend_mutex;

//create a connection to the backend server
int create_connection(const string& ip,int port)
{
    int backend_fd = socket(AF_INET,SOCK_STREAM,0);
    if(backend_fd == -1)
    {
        perror("socket (backend) failed");
        return -1;
    }
    sockaddr_in backend_addr{};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(port);
    inet_pton(AF_INET,ip.c_str(),&backend_addr.sin_addr); // convert a human-redable adress string into binary format

    if(connect(backend_fd,(sockaddr*)&backend_addr,sizeof(backend_addr)) == -1)
    {
        perror("connect to backend failed");
        close(backend_fd);
        return -1;
    }
    return backend_fd;
}

//handle client request
void handle_client(int client_fd) {
    int backend_fd = -1;
    int start_index;

    vector<int>try_backends; // store the index of the backend servers that are healthy
    {
        lock_guard<std::mutex> lock(backend_mutex);
        for(size_t i = 0; i < backend_servers.size(); ++i)
        {
            if(backend_health[i])
            {
                try_backends.push_back(i);
            }
        }
    }

    if(try_backends.empty())
    {
        string msg = "503 Service Unavailable\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        close(client_fd);
        return;
    }

    //round-robin select a backend server from the healthy ones
    static mutex rr_mutex;
    static size_t rr_index = 0;
    size_t start;
    {
        lock_guard<mutex>lock(rr_mutex);
        start_index = rr_index;
        rr_index = (rr_index + 1) % try_backends.size();
    }

    // Try each backend, starting from current one (round-robin)
    int index = -1;
    for (size_t i = 0; i < try_backends.size(); ++i) {
        int candidate = try_backends[(start_index + i) % try_backends.size()];
        const auto&[ip,port] = backend_servers[candidate];
        backend_fd = create_connection(ip, port);
        if (backend_fd != -1) {
            index = candidate; // Save the working backend index
            break;
        } else {
            cerr << "Backend " << ip << ":" << port << " failed. Trying next..." << std::endl;
        }
    }

    if (backend_fd == -1) {
        string msg = "503 Service Unavailable\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
        close(client_fd);
        return;
    }

    // Proxy request to working backend
    char buffer[1024];
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes > 0) {
        send(backend_fd, buffer, bytes, 0);
        ssize_t backend_bytes = recv(backend_fd, buffer, sizeof(buffer), 0);
        if (backend_bytes > 0) {
            send(client_fd, buffer, backend_bytes, 0);
        }
    }
    {
        lock_guard<mutex>lock(backend_mutex);
        if (index != -1) {
            request_count[index]++;
        }
    }

    close(client_fd);
    close(backend_fd);
}

//health check loop
void health_check_loop()
{
    while(true)
    {
        this_thread::sleep_for(chrono::seconds(5));
        for(size_t i = 0; i < backend_servers.size(); ++i)
        {
            auto[ip,port] = backend_servers[i];
            int fd = socket(AF_INET,SOCK_STREAM,0);
            if(fd == -1)
            {
                continue;
            }
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            inet_pton(AF_INET,ip.c_str(),&addr.sin_addr);
            
            //try to connect to the backend server 2s timeout
            struct timeval timeout;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
            setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));
            bool alive = connect(fd,(sockaddr*)&addr,sizeof(addr)) != -1;
            close(fd);
            
            lock_guard<mutex>lock(backend_mutex);
            backend_health[i] = alive;
        }
    

        cout << "\n health status ----👉"<< endl;
        for(size_t i=0;i<backend_servers.size();++i)
        {
            const auto&[ip,port] = backend_servers[i];
            string status = backend_health[i] ? "😌 healthy" : "🤮unhealthy";
            cout << "Backend " << ip << ":" << port << "[" << status << "]" << endl;
        }
        cout << "--------------------------------" << endl;

        cout << "\n[request count]";
        for(size_t i = 0; i < backend_servers.size(); ++i)
        {
            const auto&[ip,port] = backend_servers[i];
            cout << ip << ":" << port << "=" << request_count[i] << " " <<endl;
        }
        cout << "\n--------------------------------\n" << endl;
    }
}


int main()
{
    thread checker(health_check_loop);
    checker.detach(); // runs in background

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
    server_addr.sin_port = htons(8080);

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
    cout << "load balancer is running with multithreading on a port 8080.... " << endl;



    while(true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_len);
        if(client_fd == -1)
        {
            perror("client accept failed");
            continue;
        }
        //create a new thread for each client
        thread client_thread(handle_client,client_fd);
        client_thread.detach();//detach the thread from the main thread let it run independently

    }
    close(server_fd);
    return 0;
}