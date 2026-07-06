_This project has been created as part of the 42 curriculum by msokolov, elikymova._

# webserv

## Description

webserv is a lightweight HTTP server written in C++. It parses a configuration file, creates one or more server instances, and serves static content from the `www/` directory. The project includes request parsing, routing, CGI support, redirection handling, custom error pages, and basic method and body-size controls.

## Instructions

### Compilation

Build the project from the repository root with:

```bash
make
```

This produces the `webserv` executable.

### Execution

Run the server with the default configuration:

```bash
./webserv
```

Or provide a custom configuration file:

```bash
./webserv config/config.conf
```

The default configuration listens on port `8080` and serves files from `www/`.

### Cleanup

Remove build artifacts with:

```bash
make clean
```

Remove the executable and build artifacts with:

```bash
make fclean
```

Rebuild everything from scratch with:

```bash
make re
```

## Resources

- [Makefile](Makefile) - used to understand the build targets, compiler flags, and source layout for the compilation instructions.
- [config/config.conf](config/config.conf) - used as the default runtime configuration and as a reference for server blocks, routes, CGI, and error pages.
- [MDN HTTP overview](https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview) - useful for understanding HTTP requests, responses, methods, and status codes implemented by the server.
- [cppreference](https://en.cppreference.com/) - useful while implementing and reviewing the C++ standard library features used throughout the server code.
- [HTTP/1.1 semantics](https://www.rfc-editor.org/rfc/rfc9110) - useful for routing, response codes, redirects, and request handling behavior.
