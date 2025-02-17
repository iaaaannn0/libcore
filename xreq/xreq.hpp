#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <future>
#include <curl/curl.h>

namespace xreq {
    class Response {
    public:
        int status_code = 0;
        std::string text;
        std::map<std::string, std::string> headers;
    };

    class Request {
    public:
        explicit Request(const std::string& url);
        ~Request();
        

        Request& Header(const std::string& key, const std::string& value);
        Request& Param(const std::string& key, const std::string& value);
        Request& Timeout(int ms);
        Request& Insecure(bool enable = true);
        Request& CookieFile(const std::string& filename);
        Request& Body(const std::string& data, const std::string& content_type = "text/plain");
        Request& File(const std::string& field_name, const std::string& file_path);
        

        Response Send();
        std::future<Response> SendAsync();


        static Response Get(const std::string& url);
        static Response Post(const std::string& url, const std::string& data);
        
    private:
        CURL* curl;
        std::string url;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> params;
        std::string post_data;
        curl_mime* mime_form = nullptr;
        long timeout_ms = 3000;
        bool verify_ssl = true;
        std::string cookie_file;

        void SetupCommonOptions();
        void SetupMimeForm();
        static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
        static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
    };
}
