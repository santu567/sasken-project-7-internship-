# C++ Load Balancer 
<--------sasken project -> 7-------------->



A robust intermediate-level load balancer written in C++ with health monitoring, round-robin load balancing, and multi-threading capabilities.

## 🚀 Features

- ✅ **Multi-threading**: Handles multiple clients concurrently
- ✅ **Health Monitoring**: Active health checks every 5 seconds with 2-second timeout
- ✅ **Round-Robin Load Balancing**: Distributes requests among healthy backends
- ✅ **Fault Tolerance**: Gracefully handles backend failures
- ✅ **Real-time Monitoring**: Displays health status and request counts
- ✅ **Thread Safety**: Proper mutex usage for shared state
- ✅ **Error Handling**: Comprehensive socket error handling

## 📁 Project Structure

```
final_project/
├── load_balancer.cpp      # Main load balancer implementation
├── backend_server.cpp      # Simple echo backend server
└── README.md              # This file
```

## 🛠️ Compilation

### Prerequisites
- GCC with C++17 support
- POSIX-compliant system (Linux/macOS)

### Build Commands

```bash
# Compile load balancer
g++ -std=c++17 -pthread -o load_balancer load_balancer.cpp

# Compile backend server
g++ -std=c++17 -o backend_server backend_server.cpp

# Or compile both at once
g++ -std=c++17 -pthread -o load_balancer load_balancer.cpp
g++ -std=c++17 -o backend_server backend_server.cpp
```

## 🚀 Quick Start

### 1. Start Backend Servers

Open multiple terminal windows and run:

```bash
# Terminal 1
./backend_server 9001

# Terminal 2  
./backend_server 9002

# Terminal 3
./backend_server 9003
```

### 2. Start Load Balancer

```bash
# Terminal 4
./load_balancer
```

### 3. Test the System

```bash
# Test with curl
curl http://localhost:8080/hello

# Test multiple requests
for i in {1..10}; do
  curl http://localhost:8080/test$i
done
```

## 📊 Architecture

### Load Balancer Components

1. **Backend Configuration**
   ```cpp
   vector<pair<string,int>> backend_servers = {
       {"127.0.0.1", 9001},
       {"127.0.0.1", 9002},
       {"127.0.0.1", 9003}
   };
   ```

2. **Health Monitoring**
   - Checks every 5 seconds
   - 2-second connection timeout
   - Updates health status in real-time

3. **Load Balancing Algorithm**
   - Round-robin among healthy backends
   - Skips unhealthy servers automatically
   - Thread-safe selection

4. **Multi-threading**
   - Each client gets its own thread
   - Detached threads for independent execution
   - Background health check thread

## 🔧 Configuration

### Current Backend Servers
- **Backend 1**: 127.0.0.1:9001
- **Backend 2**: 127.0.0.1:9002  
- **Backend 3**: 127.0.0.1:9003

### Load Balancer Settings
- **Port**: 8080
- **Health Check Interval**: 5 seconds
- **Connection Timeout**: 2 seconds
- **Backlog**: 5 pending connections

## 📈 Monitoring

The load balancer provides real-time monitoring output:

```
 health status ----👉
Backend 127.0.0.1:9001 [😌 healthy]
Backend 127.0.0.1:9002 [😌 healthy]
Backend 127.0.0.1:9003 [🤮unhealthy]
--------------------------------

[request count]
127.0.0.1:9001=15 
127.0.0.1:9002=8 
127.0.0.1:9003=0 
--------------------------------
```

## 🔍 How It Works

### 1. Client Request Flow
```
Client → Load Balancer → Round-Robin Selection → Healthy Backend → Response
```

### 2. Health Check Process
```
Every 5 seconds:
├── Try connect to each backend (2s timeout)
├── Update health status
├── Display status and request counts
└── Continue monitoring
```

### 3. Load Balancing Logic
```
1. Get list of healthy backends
2. Apply round-robin selection
3. Try to connect to selected backend
4. If fails, try next healthy backend
5. If all fail, return 503 Service Unavailable
6. Proxy request to working backend
7. Update request count
```

## 🛡️ Error Handling

- **Backend Failures**: Automatically detected and bypassed
- **Connection Timeouts**: 2-second timeout for health checks
- **Socket Errors**: Proper cleanup and error reporting
- **No Healthy Backends**: Returns 503 Service Unavailable
- **Client Disconnections**: Graceful handling

## 🧪 Testing

### Manual Testing
```bash
# Start all components
./backend_server 9001 &
./backend_server 9002 &
./backend_server 9003 &
./load_balancer &

# Test with curl
curl http://localhost:8080/hello
curl http://localhost:8080/world
curl http://localhost:8080/test

# Kill a backend to test fault tolerance
pkill -f "backend_server 9002"
```

### Expected Behavior
- Requests should be distributed across healthy backends
- When a backend fails, requests continue to healthy ones
- Health status should update in real-time
- Request counts should increment per backend

## 📋 Code Quality Assessment

### Intermediate Level Features ✅
- Multi-threading with proper synchronization
- Health monitoring with timeouts
- Round-robin load balancing
- Fault tolerance and error handling
- Real-time monitoring and statistics

### Advanced Features (Future Enhancements)
- [ ] Dynamic configuration (file-based)
- [ ] Multiple load balancing algorithms
- [ ] Connection pooling
- [ ] SSL/TLS support
- [ ] Weighted load balancing
- [ ] API for dynamic backend management

## 🔧 Troubleshooting

### Common Issues

1. **Port Already in Use**
   ```bash
   # Check what's using the port
   lsof -i :8080
   # Kill the process
   kill -9 <PID>
   ```

2. **Backend Servers Not Starting**
   ```bash
   # Check if ports are available
   netstat -an | grep 9001
   ```

3. **Load Balancer Not Responding**
   ```bash
   # Check if load balancer is running
   ps aux | grep load_balancer
   ```

### Debug Mode
Add debug output by modifying the code:
```cpp
// Add debug prints
cout << "Debug: Trying backend " << ip << ":" << port << endl;
```

## 📚 Technical Details

### Dependencies
- `<thread>`: Multi-threading support
- `<mutex>`: Thread synchronization
- `<chrono>`: Time-based operations
- `<sys/socket.h>`: Socket programming
- `<netinet/in.h>`: Internet address structures

### Threading Model
- **Main Thread**: Accepts client connections
- **Client Threads**: Handle individual client requests (detached)
- **Health Check Thread**: Monitors backend health (detached)

### Memory Management
- Proper socket cleanup with `close()`
- RAII-style mutex usage with `lock_guard`
- Automatic thread cleanup with `detach()`

## 🎯 Performance Characteristics

- **Concurrent Clients**: Unlimited (limited by system resources)
- **Health Check Overhead**: Minimal (2s timeout per backend)
- **Request Latency**: Low (direct proxy)
- **Memory Usage**: Constant (no connection pooling)

## 📝 License

This is a learning project demonstrating intermediate C++ network programming concepts.

---

**Note**: This load balancer is designed for educational purposes and demonstrates intermediate-level C++ programming concepts including multi-threading, socket programming, and system-level programming. 