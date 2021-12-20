//
// Created by mortacious on 12/19/21.
//

#include "vtkPotree1_7DatasetUrl.h"
#include <sstream>
#include <streambuf>

static size_t write_function(void *buffer, size_t size, size_t nmemb, void *param)
{
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

vtkPotree1_7DatasetUrl::vtkPotree1_7DatasetUrl()
: vtkPotree1_7DatasetBase() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

vtkPotree1_7DatasetUrl::~vtkPotree1_7DatasetUrl() {
    curl_global_cleanup();

}

std::string vtkPotree1_7DatasetUrl::FetchFile(const std::string &filename) const {
    std::string result;
    //result.reserve(100000);
    CURLcode res;
    CURL* Curl = curl_easy_init();
    if(Curl) {
        //std::cout << "Debug: " << istringstreamurl.c_str() << std::endl;
        curl_easy_setopt(Curl, CURLOPT_URL, filename.c_str());
        //curl_easy_setopt(Curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.153 Safari/537.36");
        curl_easy_setopt(Curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(Curl, CURLOPT_FAILONERROR, true);
        // Define our callback to get called when there's data to be written
        curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, write_function);
        // Set a pointer to our struct to pass to the callback
        curl_easy_setopt(Curl, CURLOPT_WRITEDATA, &result);

        /* Switch on full protocol/debug output */
        //curl_easy_setopt(Curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(Curl);

        if(res != CURLE_OK) {
            // we failed
            std::stringstream ss;
            ss << "Error from curl on url: " << filename << ": " << res;
            throw std::runtime_error(ss.str());
        }
        /* always cleanup */
        curl_easy_cleanup(Curl);
    }

    return std::move(result);
}