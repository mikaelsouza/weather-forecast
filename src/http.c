#include "../include/http.h"
#include "../include/core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

static int str_contains_case_insensitive(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    size_t haystack_len = strlen(haystack);
    size_t needle_len = strlen(needle);
    if (needle_len > haystack_len) return 0;
    
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        int match = 1;
        for (size_t j = 0; j < needle_len; j++) {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j])) {
                match = 0;
                break;
            }
        }
        if (match) return 1;
    }
    return 0;
}

#define BUFFER_SIZE 65536

static int parse_url(const char* url, char** host, char** path, int* port) {
    const char* start = url;
    
    // Skip http://
    if (strncmp(start, "http://", 7) == 0) {
        start += 7;
    } else if (strncmp(start, "https://", 8) == 0) {
        fprintf(stderr, "HTTPS not supported\n");
        return -1;
    }
    
    // Find path separator
    const char* slash = strchr(start, '/');
    const char* colon = strchr(start, ':');
    
    // Extract host and port
    int host_len;
    if (colon && (!slash || colon < slash)) {
        host_len = colon - start;
        *port = atoi(colon + 1);
    } else {
        host_len = slash ? (slash - start) : strlen(start);
        *port = 80;
    }
    
    *host = malloc_safe(host_len + 1);
    memcpy(*host, start, host_len);
    (*host)[host_len] = '\0';
    
    // Extract path
    if (slash) {
        *path = strdup_safe(slash);
    } else {
        *path = strdup_safe("/");
    }
    
    return 0;
}

static char* decode_chunked(const char* body) {
    size_t total_size = 0;
    char* result = malloc_safe(1);
    result[0] = '\0';
    
    const char* p = body;
    while (*p) {
        // Read chunk size
        char* end;
        long chunk_size = strtol(p, &end, 16);
        if (p == end) break;
        
        p = end;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
        
        if (chunk_size == 0) break;
        
        // Append chunk to result
        result = realloc(result, total_size + chunk_size + 1);
        memcpy(result + total_size, p, chunk_size);
        total_size += chunk_size;
        result[total_size] = '\0';
        
        p += chunk_size;
        if (*p == '\r') p++;
        if (*p == '\n') p++;
    }
    
    return result;
}

char* http_get(const char* url) {
    char* host = NULL;
    char* path = NULL;
    int port;
    
    if (parse_url(url, &host, &path, &port) < 0) {
        return NULL;
    }
    
    // Resolve hostname
    struct hostent* server = gethostbyname(host);
    if (!server) {
        fprintf(stderr, "Failed to resolve %s\n", host);
        free(host);
        free(path);
        return NULL;
    }
    
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        free(host);
        free(path);
        return NULL;
    }
    
    // Connect
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        free(host);
        free(path);
        return NULL;
    }
    
    // Send HTTP request
    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, host);
    
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("write");
        close(sockfd);
        free(host);
        free(path);
        return NULL;
    }
    
    // Read response
    size_t capacity = BUFFER_SIZE;
    char* response = malloc_safe(capacity);
    size_t total_read = 0;
    ssize_t n;
    
    while ((n = read(sockfd, response + total_read, capacity - total_read - 1)) > 0) {
        total_read += n;
        if (total_read >= capacity - 1) {
            capacity *= 2;
            response = realloc(response, capacity);
        }
    }
    
    response[total_read] = '\0';
    close(sockfd);
    free(host);
    free(path);
    
    // Find body (after \r\n\r\n)
    char* body = strstr(response, "\r\n\r\n");
    if (!body) {
        free(response);
        return NULL;
    }
    
    char* headers = response;
    *body = '\0'; // Split headers and body
    body += 4;
    
    // Check status
    if (strncmp(headers, "HTTP/1", 6) != 0 || !strstr(headers, "200")) {
        fprintf(stderr, "HTTP request failed: %s\n", headers);
        free(response);
        return NULL;
    }
    
    char* result;
    // Check for chunked encoding
    if (str_contains_case_insensitive(headers, "Transfer-Encoding: chunked")) {
        result = decode_chunked(body);
    } else {
        result = strdup_safe(body);
    }
    
    free(response);
    return result;
}
