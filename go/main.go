package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"time"

	"gocv.io/x/gocv"
)

type ImagePair struct {
	BaseURL string `json:"baseURL"`
	CompURL string `json:"compURL"`
}

type InputData struct {
	Urls []ImagePair `json:"urls"`
}

// downloadImage downloads an image from a URL and decodes it into a gocv.Mat
func downloadImage(url string) (gocv.Mat, error) {
	client := &http.Client{
		Timeout: 30 * time.Second,
	}

	resp, err := client.Get(url)
	if err != nil {
		return gocv.NewMat(), fmt.Errorf("error downloading image from %s: %v", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return gocv.NewMat(), fmt.Errorf("failed to download image from %s: status code %d", url, resp.StatusCode)
	}

	data, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return gocv.NewMat(), fmt.Errorf("error reading response body from %s: %v", url, err)
	}

	img, err := gocv.IMDecode(data, gocv.IMReadColor)
	if err != nil {
		return gocv.NewMat(), fmt.Errorf("error decoding image from %s: %v", url, err)
	}

	return img, nil
}

func compareImages(img1, img2 gocv.Mat) int {

	mismatchedPixels := 0
	rows, cols, channels := img1.Rows(), img1.Cols(), img1.Channels()

	// GO_CV version
	// for y := 0; y < rows; y++ {
	// 	for x := 0; x < cols; x++ {

	// 		pixel1 := img1.GetVecbAt(y, x)
	// 		pixel2 := img2.GetVecbAt(y, x)
	// 		mismatch := false
	// 		for c := 0; c < channels; c++ {
	// 			if pixel1[c] != pixel2[c] {
	// 				mismatch = true
	// 				break
	// 			}
	// 		}
	// 		if mismatch {
	// 			mismatchedPixels++
	// 		}
	// 	}
	// }

	// DIRECT_PIXEL
	data1 := img1.ToBytes()
	data2 := img2.ToBytes()

	// Iterate over raw pixel data
	for i := 0; i < rows*cols*channels; i += channels {
		mismatch := false
		for c := 0; c < channels; c++ {
			if data1[i+c] != data2[i+c] {
				mismatch = true
				break
			}
		}
		if mismatch {
			mismatchedPixels++
		}
	}

	return mismatchedPixels
}

func main() {
	// Load JSON file
	file, err := os.Open("urls.json")
	if err != nil {
		fmt.Println("Error opening JSON file:", err)
		return
	}
	defer file.Close()

	var input InputData
	if err := json.NewDecoder(file).Decode(&input); err != nil {
		fmt.Println("Error decoding JSON:", err)
		return
	}

	// Load images into memory
	var imagePairs []struct {
		Base, Comp gocv.Mat
	}
	counter := 0
	for _, pair := range input.Urls {
		if counter > 10 {
			break
		}
		counter++

		img1, err := downloadImage(pair.BaseURL)
		if err != nil {
			fmt.Println("Error downloading base image:", err)
			return
		}
		img2, err := downloadImage(pair.CompURL)
		if err != nil {
			fmt.Println("Error downloading comparison image:", err)
			return
		}
		imagePairs = append(imagePairs, struct {
			Base, Comp gocv.Mat
		}{img1, img2})
	}

	fmt.Println("Image loading completed")

	// Benchmark comparison
	start := time.Now()

	for _, pair := range imagePairs {
		_ = compareImages(pair.Base, pair.Comp)
	}

	duration := time.Since(start)
	fmt.Printf("Total Execution Time: %.6f seconds\n", duration.Seconds())
}
