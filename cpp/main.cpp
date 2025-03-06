#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

 size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::vector<uchar>* data = static_cast<std::vector<uchar>*>(userp);
    data->insert(data->end(), (uchar*)contents, (uchar*)contents + totalSize);
    return totalSize;
}

bool downloadImage(const std::string& url, std::vector<uchar>& imageData) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing CURL.\n";
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // Write data callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    // Data pointer
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageData);
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_easy_cleanup(curl);
        return false;
    }
    curl_easy_cleanup(curl);
    return true;
}

int compareImages(const cv::Mat& img1, const cv::Mat& img2) {
    int mismatchedPixels = 0;

    int width = img1.cols;
    int height = img1.rows;
    int channels = img1.channels();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            if (y >= img1.rows || x >= img1.cols || y >= img2.rows || x >= img2.cols) {
                continue; 
            }
        
            const uchar* pixel1 = img1.ptr<uchar>(y) + x * channels;
            const uchar* pixel2 = img2.ptr<uchar>(y) + x * channels;

            bool mismatch = false;
            for (int c = 0; c < channels; ++c) {
                if (pixel1[c] != pixel2[c]) {
                    mismatch = true;
                    break;
                }
            }
            if (mismatch) {
                mismatchedPixels++;
            }
        }
    }

    for (int i =0; i < )

    return mismatchedPixels;
}

int main(int argc, char* argv[]) {
  
    // Load JSON file
    std::ifstream jsonFile("urls.json");
    if (!jsonFile.is_open()) {
        std::cerr << "Failed to open JSON file.\n";
        return EXIT_FAILURE;
    }

    json jsonData;
    jsonFile >> jsonData;

    std::vector<std::pair<cv::Mat, cv::Mat>> imagePairs;

    // Load images into memory
    curl_global_init(CURL_GLOBAL_DEFAULT);
    int counter = 0;
    for (const auto& pair : jsonData["urls"]) {
        if (counter > 10) break;
        counter++;

        std::vector<uchar> imageData1, imageData2;
        if (!downloadImage(pair["baseURL"], imageData1)) {
            std::cerr << "Failed to download base image.\n";
            return EXIT_FAILURE;
        }
        if (!downloadImage(pair["compURL"], imageData2)) {
            std::cerr << "Failed to download comparison image.\n";
            return EXIT_FAILURE;
        }

        cv::Mat img1 = cv::imdecode(imageData1, cv::IMREAD_COLOR);
        cv::Mat img2 = cv::imdecode(imageData2, cv::IMREAD_COLOR);

        if (img1.empty() || img2.empty()) {
            std::cerr << "Failed to decode images.\n";
            return EXIT_FAILURE;
        }
        imagePairs.emplace_back(img1, img2);
    }
    std::cout << "Loading completed " << imagePairs.size() << std::endl; 

    // Benchmark pixel comparison
    auto start = std::chrono::high_resolution_clock::now();
    int sum = 0;

    for (const auto& [img1, img2] : imagePairs) {
        int mismatchedPixels = countMismatchedPixels(img1, img2);
        sum += mismatchedPixels;
    }
    std::cout << sum << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Total Time Taken: " << diff.count() << " ms\n";

    curl_global_cleanup();
    return 0;
}
