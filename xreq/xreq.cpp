#include "xreq.hpp"
#include <sstream>
#include <iostream>

namespace xreq {
    
    size_t Request::WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        Response* response = static_cast<Response*>(userdata);
        response->text.append(ptr, size * nmemb);
        return size * nmemb;
    }

    
    size_t Request::HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
        Response* response = static_cast<Response*>(userdata);
        std::string header_line(buffer, size * nitems);
        size_t colon = header_line.find(':');
        if (colon != std::string::npos) {
            std::string key = header_line.substr(0, colon);
            std::string value = header_line.substr(colon + 2); // 跳过冒号和空格
            response->headers[key] = value;
        }
        return size * nitems;
    }

    Request::Request(const std::string& url) : url(url) {
        curl = curl_easy_init();
    }

    Request::~Request() {
        if (mime_form) curl_mime_free(mime_form);
        if (curl) curl_easy_cleanup(curl);
    }

    Request& Request::Header(const std::string& key, const std::string& value) {
        headers[key] = value;
        return *this;
    }

    Request& Request::Param(const std::string& key, const std::string& value) {
        params[key] = value;
        return *this;
    }

    Request& Request::Timeout(int ms) {
        timeout_ms = ms;
        return *this;
    }

    Request& Request::Insecure(bool enable) {
        verify_ssl = !enable;
        return *this;
    }

    Request& Request::CookieFile(const std::string& filename) {
        cookie_file = filename;
        return *this;
    }

    Request& Request::Body(const std::string& data, const std::string& content_type) {
        post_data = data;
        Header("Content-Type", content_type);
        return *this;
    }

    Request& Request::File(const std::string& field_name, const std::string& file_path) {
        if (!mime_form) {
            mime_form = curl_mime_init(curl);
        }
        curl_mimepart* part = curl_mime_addpart(mime_form);
        curl_mime_name(part, field_name.c_str());
        curl_mime_filedata(part, file_path.c_str());
        return *this;
    }

    void Request::SetupCommonOptions() {
        
        if (!params.empty()) {
            std::ostringstream oss;
            oss << url << "?";
            for (auto& [key, value] : params) {
                char* encoded = curl_easy_escape(curl, value.c_str(), value.length());
                oss << key << "=" << encoded << "&";
                curl_free(encoded);
            }
            url = oss.str();
            url.pop_back(); 
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
        
        
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_ssl ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_ssl ? 2L : 0L);

        
        if (!cookie_file.empty()) {
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookie_file.c_str());
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookie_file.c_str());
        }
    }

    void Request::SetupMimeForm() {
        if (mime_form) {
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime_form);
        } else if (!post_data.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        }
    }

    Response Request::Send() {
        Response response;
        struct curl_slist* curl_headers = nullptr;

        SetupCommonOptions();

        
        for (auto& [key, value] : headers) {
            curl_headers = curl_slist_append(curl_headers, (key + ": " + value).c_str());
        }
        if (curl_headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

        
        SetupMimeForm();

       
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);
        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
        } else {
            response.text = curl_easy_strerror(res);
        }

        curl_slist_free_all(curl_headers);
        return response;
    }

    std::future<Response> Request::SendAsync() {
        return std::async(std::launch::async, [this]() {
            return this->Send();
        });
    }

    Response Request::Get(const std::string& url) {
        return Request(url).Send();
    }

    Response Request::Post(const std::string& url, const std::string& data) {
        return Request(url).Body(data, "application/x-www-form-urlencoded").Send();
    }
}
