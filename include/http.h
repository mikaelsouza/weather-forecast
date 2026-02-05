#ifndef HTTP_H
#define HTTP_H

// Perform HTTP GET request
// Returns response body (caller must free)
// Returns NULL on error
char* http_get(const char* url);

#endif // HTTP_H
