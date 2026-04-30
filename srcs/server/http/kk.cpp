#include "HttpResponse.hpp"
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>

// -------------------- MIME TYPE --------------------

std::string getMimeType(const std::string& path)
{
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html")
        return "text/html; charset=utf-8";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css")
        return "text/css; charset=utf-8";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")
        return "image/png";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg")
        return "image/jpeg";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".ico")
        return "image/x-icon";
    return "application/octet-stream";
}

// -------------------- READ FILE --------------------

std::string readFile(const std::string& filePath)
{
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open())
        return "";

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

// -------------------- BUILD RESPONSE --------------------

std::string HTTPResponse::build(const HTTPRequest& request)
{
    std::string path = request._path;
    std::string filePath;

    // 1. PATH → FILE
    if (path == "/")
        filePath = "www/index.html";
    else
        filePath = "www" + path;

    std::cout << "Serving: " << filePath << std::endl;

    // 2. READ FILE
    std::string body = readFile(filePath);
    
    // Debug: show if file was found
    if (body.empty())
        std::cout << "  ⚠️  File NOT FOUND or empty!" << std::endl;
    else
        std::cout << "  ✓ Loaded " << body.size() << " bytes" << std::endl;

    // 3. HANDLE 404
    std::string status;
    std::string contentType;

    if (body.empty())
    {
        status = "HTTP/1.1 404 Not Found\r\n";
        body = "<h1>404 Not Found</h1>";
        contentType = "text/html";
    }
    else
    {
        status = "HTTP/1.1 200 OK\r\n";
        contentType = getMimeType(filePath);
    }

    // 4. BUILD RESPONSE
    std::string response =
        status +
        "Content-Type: " + contentType + "\r\n" +
        "Content-Length: " + std::to_string(body.size()) + "\r\n" +
        "Cache-Control: no-cache\r\n" +
        "Connection: close\r\n" +
        "\r\n" +
        body;

    return response;
}