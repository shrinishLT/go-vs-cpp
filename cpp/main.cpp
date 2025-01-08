#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

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

int countMismatchedPixels(const cv::Mat& img1, const cv::Mat& img2) {
    
    int rows = img1.rows;
    int cols = img1.cols;
    int channels = img1.channels();
    int mismatchedPixels = 0;

    const uchar* ptr1 = img1.ptr<uchar>(0);
    const uchar* ptr2 = img2.ptr<uchar>(0);
    size_t totalPixels = static_cast<size_t>(rows) * cols;


    for (size_t i = 0; i < totalPixels; ++i) {
        bool mismatch = false;
        for (int c = 0; c < channels; ++c) {
            if (ptr1[i * channels + c] != ptr2[i * channels + c]) {
                mismatch = true;
                break;
            }
        }
        if (mismatch) {
            mismatchedPixels++;
        }
    }

    return mismatchedPixels;
}

int main(int argc, char* argv[]) {

    
    auto start = std::chrono::high_resolution_clock::now();

    std::string url1 = argv[1];
    std::string url2 = argv[2];
    std::vector<uchar> imageData1, imageData2;

     
    curl_global_init(CURL_GLOBAL_DEFAULT);

   
    if (!downloadImage(url1, imageData1)) {
        std::cerr << "Failed to download image from " << url1 << "\n";
        return EXIT_FAILURE;
    }


    if (!downloadImage(url2, imageData2)) {
        std::cerr << "Failed to download image from " << url2 << "\n";
        return EXIT_FAILURE;
    }


    cv::Mat img1 = cv::imdecode(imageData1, cv::IMREAD_COLOR);
    cv::Mat img2 = cv::imdecode(imageData2, cv::IMREAD_COLOR);

    if (img1.empty()) {
        std::cerr << "Failed to decode image from " << url1 << "\n";
        return EXIT_FAILURE;
    }

    if (img2.empty()) {
        std::cerr << "Failed to decode image from " << url2 << "\n";
        return EXIT_FAILURE;
    }

    int mismatchedPixels = countMismatchedPixels(img1, img2);
    std::cout << mismatchedPixels << std::endl;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;


    std::cout << "Mismatched Pixels: " << mismatchedPixels << "\n";
    std::cout << "Time Taken in cpp: " << diff.count() << " seconds\n";


    start = std::chrono::high_resolution_clock::now();

    for(int i=0;i<2;i++){
        int mismatchedPixels = countMismatchedPixels(img1, img2);
    }

    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "Time Taken in cpp for 100 screenshots : " << diff.count() << " seconds\n";

    curl_global_cleanup();
    return 0;
}
