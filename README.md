# WebServer

A high-performance HTTP/1.1 web server implementation written in C++20, featuring event-driven architecture with epoll, CGI support, and flexible configuration management.

## Features

- **HTTP/1.1 Protocol Support**: Full implementation of HTTP/1.1 with support for persistent connections
- **Multiple HTTP Methods**: GET, POST, and DELETE request handling
- **CGI Script Execution**: Support for Python and PHP CGI scripts
- **Static File Serving**: Efficient serving of static files with proper MIME type detection
- **File Upload/Download**: Complete file management capabilities
- **Directory Listing**: Automatic directory indexing when enabled
- **Configuration-Based**: Flexible server configuration using .conf files
- **Multi-Server Support**: Run multiple server instances with different configurations
- **Error Handling**: Custom error pages and proper HTTP status codes
- **URL Redirection**: Support for HTTP redirections
- **Event-Driven Architecture**: High-performance epoll-based I/O handling
- **Request Size Limiting**: Configurable client body size limits
- **Comprehensive Logging**: Built-in logging system for monitoring and debugging

## Prerequisites

- **C++ Compiler**: Supporting C++20 standard (GCC 10+ or Clang 10+)
- **Make**: Build automation tool
- **Linux/Unix System**: Required for epoll functionality
- **Python**: For CGI script testing (optional)
- **PHP**: For PHP CGI script testing (optional)

## Building

1. **Clone the repository**:
   ```bash
   git clone https://github.com/omartela/WebServer.git
   cd WebServer
   ```

2. **Build the project**:
   ```bash
   make
   ```

3. **Clean build artifacts** (optional):
   ```bash
   make clean    # Remove object files
   make fclean   # Remove object files and executable
   make re       # Clean and rebuild
   ```

## Usage

### Basic Usage

```bash
./webserver <configuration_file>
```

Example:
```bash
./webserver configurationfiles/simple.conf
```

### Configuration File Format

The server uses `.conf` files for configuration. Here's a basic example:

```conf
server {
    listen 127.0.0.1:8080;
    server_name example.com www.example.com;
    client_max_body_size 10M;
    error_page 404 /404.html;

    location / {
        abspath /www/;
        index index.html;
        autoindex on;
        allow_methods GET POST;
    }

    location /uploads {
        abspath /www/uploads/;
        allow_methods GET POST DELETE;
        upload_path /www/uploads/;
    }

    location /cgi {
        abspath /www/cgi/;
        allow_methods GET POST;
        cgi_extension .py .php;
        cgipathpython /usr/bin/python3;
        cgipathphp /usr/bin/php;
    }
}
```

### Configuration Directives

#### Server Block Directives

- **`listen`**: IP address and port (e.g., `127.0.0.1:8080`)
- **`server_name`**: Domain names for the server
- **`client_max_body_size`**: Maximum upload size (supports K, M suffixes)
- **`error_page`**: Custom error pages (e.g., `404 /404.html`)

#### Location Block Directives

- **`abspath`**: Absolute path to the location directory
- **`index`**: Default index file
- **`autoindex`**: Enable/disable directory listing (`on`/`off`)
- **`allow_methods`**: Allowed HTTP methods (`GET`, `POST`, `DELETE`)
- **`return`**: HTTP redirection (e.g., `301 /new-location`)
- **`cgi_extension`**: File extensions for CGI scripts
- **`upload_path`**: Directory for file uploads
- **`cgipathpython`**: Path to Python interpreter
- **`cgipathphp`**: Path to PHP interpreter

## Testing

The project includes comprehensive test suites:

### Python Test Scripts

Located in the `tests/` directory:

```bash
# Run basic functionality tests
python3 tests/test1.py

# Run CGI tests
python3 tests/cgitest/request_test.py

# Run unit tests
python3 tests/python_unit_tests.py
```

### Manual Testing

1. **Start the server**:
   ```bash
   ./webserver configurationfiles/simple.conf
   ```

2. **Test with curl**:
   ```bash
   # GET request
   curl http://127.0.0.2:8004/

   # POST request with file upload
   curl -X POST -F "file=@test.txt" http://127.0.0.2:8004/uploads/

   # DELETE request
   curl -X DELETE http://127.0.0.2:8004/uploads/test.txt
   ```

3. **Test with web browser**:
   Navigate to `http://127.0.0.2:8004/` to see the web interface.

## Project Structure

```
WebServer/
├── Makefile                 # Build configuration
├── README.md               # Project documentation
├── configurationfiles/     # Example configuration files
│   ├── simple.conf         # Basic server configuration
│   ├── cgi_test.conf      # CGI testing configuration
│   └── configuration_file_rules.conf  # Configuration syntax guide
├── includes/               # Header files
│   ├── EventLoop.hpp       # Event loop management
│   ├── HTTPRequest.hpp     # HTTP request handling
│   ├── HTTPResponse.hpp    # HTTP response handling
│   ├── CGIHandler.hpp      # CGI script execution
│   ├── Client.hpp          # Client connection management
│   ├── Parser.hpp          # Configuration file parsing
│   ├── RequestHandler.hpp  # Request routing and handling
│   ├── Logger.hpp          # Logging functionality
│   └── utils.hpp           # Utility functions
├── srcs/                   # Source code
│   ├── main.cpp            # Application entry point
│   ├── utils.cpp           # Utility implementations
│   ├── HTTP/               # HTTP handling modules
│   ├── configparser/       # Configuration parsing
│   ├── epoll/             # Event loop implementation
│   └── logger/            # Logging implementation
├── tests/                  # Test suites
│   ├── test1.py           # Basic functionality tests
│   ├── cgitest/           # CGI-specific tests
│   └── python_unit_tests.py  # Unit tests
└── www/                   # Default web content
    ├── index.html         # Default homepage
    ├── cgi/              # CGI scripts
    ├── images/           # Static images
    └── error/            # Error pages
```

## Architecture

### Core Components

1. **EventLoop**: Manages epoll-based event handling for high-performance I/O
2. **HTTPRequest/HTTPResponse**: Parse and generate HTTP messages
3. **CGIHandler**: Execute and manage CGI scripts
4. **Parser**: Parse and validate configuration files
5. **Client**: Manage individual client connections
6. **Logger**: Provide comprehensive logging functionality

### Event-Driven Design

The server uses Linux's epoll mechanism for efficient handling of multiple concurrent connections, making it suitable for high-traffic scenarios.

## Example Configurations

### Simple Static Server

```conf
server {
    listen 127.0.0.1:8080;
    server_name localhost;

    location / {
        abspath /www/;
        index index.html;
        allow_methods GET;
        autoindex on;
    }
}
```

### CGI-Enabled Server

```conf
server {
    listen 127.0.0.1:8080;
    server_name localhost;
    client_max_body_size 5M;

    location / {
        abspath /www/;
        index index.html;
        allow_methods GET POST;
    }

    location /cgi-bin {
        abspath /www/cgi/;
        allow_methods GET POST;
        cgi_extension .py .php;
        cgipathpython /usr/bin/python3;
    }
}
```

## Error Handling

The server provides comprehensive error handling with:

- HTTP status code compliance
- Custom error pages
- Graceful handling of malformed requests
- Proper resource cleanup
- Detailed error logging

## Performance Features

- **Epoll-based I/O**: Efficient handling of thousands of concurrent connections
- **Non-blocking operations**: Prevents server blocking on slow clients
- **Connection reuse**: HTTP/1.1 persistent connections
- **Efficient file serving**: Optimized static file delivery
- **Memory management**: Careful resource allocation and cleanup

## License

This project is part of a web server implementation exercise. Please check with the repository owner for specific licensing terms.

## Authors

- jlehtone
- Tsaaril
- Omartela

## Acknowledgments

Built as part of a systems programming project focusing on network programming, HTTP protocol implementation, and high-performance server architecture.
