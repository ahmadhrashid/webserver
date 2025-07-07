# Multithreaded C Web Server

A minimal HTTP/1.0 web server written in C, designed to serve static files concurrently using a configurable thread pool. This project implements the following features:

1. **Basic Server**: Opens a listening socket, accepts a single connection, parses a GET request, and returns a hardcoded response.
2. **Static File Serving**: Maps URL paths to files under a document root and returns their contents with appropriate headers.
3. **Multithreading**: Uses a thread pool and thread-safe queue so multiple clients can be served simultaneously.
4. **Logging**: Records every request in `access.log` and server errors in `error.log` with timestamps, client IP, request line, and status codes.
5. **Runtime Configuration**: Allows customizing the listening port, document root, and thread count via command-line flags (`-p`, `-r`, `-t`, `-c`) or an optional config file.

---

## Dependencies

* POSIX-compatible OS (Linux, macOS)
* GCC (or clang) with pthread support

---

## Build

```bash
make
```

This will compile sources into `webserver` and place object files in `build/`.

---

## Usage

### Default

```bash
./webserver
```

* **Port**: 8080
* **Document root**: `www/`
* **Threads**: 4

### Command-Line Flags

* `-p, --port <port>`        Listening port (default 8080)
* `-r, --root <directory>`   Document root (default `www/`)
* `-t, --threads <count>`    Number of worker threads (default 4)
* `-c, --config <file>`      Optional config file
* `-h, --help`               Show help message

### Config File

Create a file named `webserver.conf` with lines in `key=value` form:

```text
port=8000
root=public_html/
threads=6
```

Then run:

```bash
./webserver -c webserver.conf
```

---

## Project Structure

```plaintext
webserver/
├── src/                # C source files and headers
│   ├── config.c, config.h
│   ├── globals.c, globals.h
│   ├── logger.c, logger.h
│   ├── queue.c, queue.h
│   ├── server.c, server.h
│   ├── threadpool.c, threadpool.h
│   └── ... (future modules)
├── www/                # Default document root (static files)
│   └── index.html
├── build/              # Compiled object files
├── Makefile            # Build instructions
├── .gitignore          # Ignored files (e.g., build/, logs)
├── webserver.conf      # Sample config file
└── README.md           # Project overview and usage
```

---

## .gitignore

```text
build/
access.log
error.log
```

---

## Future Work

* Directory listing with secure path sanitization
* Conditional GET (`If-Modified-Since`) and basic caching
* HTTPS support (OpenSSL integration)
* Response compression (gzip)
* Dynamic content via CGI or embedded interpreters
