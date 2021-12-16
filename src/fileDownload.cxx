//
// Created by mortacious on 12/16/21.
//
#include "fileDownload.h"
#include <sstream>
#include <iostream>
#include <curl/curl.h>

static size_t write_function(void *buffer, size_t size, size_t nmemb, void *param)
{
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

std::unique_ptr<std::istream> DownloadFile(const std::string& url) {
    std::string result;
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        //std::cout << "Debug: " << url.c_str() << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.153 Safari/537.36");
        //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
        // Define our callback to get called when there's data to be written
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);

        // Set a pointer to our struct to pass to the callback
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

        /* Switch on full protocol/debug output */
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);

        if(CURLE_OK != res) {
            // we failed
            std::stringstream ss;
            ss << "Error from curl: " << res;
            throw std::runtime_error(ss.str());
        }
    }
    curl_global_cleanup();

    //std::cout << "result" << result << std::endl;
    return std::make_unique<std::istringstream>(result);
}