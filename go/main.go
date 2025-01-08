package main

import (
	"flag"
	"fmt"
	"image"
	_ "image/png" // Register PNG format
	"net/http"
	"os"
	"time"
)

func toRGBA(img image.Image) *image.RGBA {
	bounds := img.Bounds()
	rgba := image.NewRGBA(bounds)
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			rgba.Set(x, y, img.At(x, y))
		}
	}
	return rgba
}

func downloadImage(url string) (image.Image, error) {

	client := &http.Client{
		Timeout: 30 * time.Second,
	}

	resp, err := client.Get(url)
	if err != nil {
		return nil, fmt.Errorf("error downloading image from %s: %v", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to download image from %s: status code %d", url, resp.StatusCode)
	}

	img, _, err := image.Decode(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("error decoding image from %s: %v", url, err)
	}

	return img, nil
}

func compareImages(img1, img2 *image.RGBA) (int, error) {

	bounds1 := img1.Bounds()

	width := bounds1.Dx()
	height := bounds1.Dy()
	stride := img1.Stride
	pix1 := img1.Pix
	pix2 := img2.Pix

	mismatchedPixels := 0
	channels := 4

	for y := 0; y < height; y++ {
		offset1 := y * stride
		offset2 := y * stride
		for x := 0; x < width; x++ {
			base1 := offset1 + x*channels
			base2 := offset2 + x*channels

			// Check if pixel indices are valid
			if base1+3 >= len(pix1) || base2+3 >= len(pix2) {
				continue // Skip invalid pixels
			}

			// Compare pixel channels
			if pix1[base1] != pix2[base2] || pix1[base1+1] != pix2[base2+1] ||
				pix1[base1+2] != pix2[base2+2] || pix1[base1+3] != pix2[base2+3] {
				mismatchedPixels++
			}
		}
	}

	return mismatchedPixels, nil
}

func main() {

	totalStart := time.Now()

	flag.Parse()

	url1 := flag.Arg(0)
	url2 := flag.Arg(1)

	img1, err := downloadImage(url1)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	img2, err := downloadImage(url2)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	mismatches, err := compareImages(toRGBA(img1), toRGBA(img2))
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error during comparison: %v\n", err)
		os.Exit(1)
	}

	totalDuration := time.Since(totalStart)

	fmt.Printf("Mismatched Pixels: %d\n", mismatches)
	fmt.Printf("Total Execution Time in go : %.6f seconds\n", totalDuration.Seconds())

	// totalStart = time.Now()

	// for i := 0; i < 101; i++ {
	// 	_, err := compareImages(toRGBA(img1), toRGBA(img2))
	// 	if err != nil {
	// 		fmt.Fprintf(os.Stderr, "Error during comparison: %v\n", err)
	// 		os.Exit(1)
	// 	}
	// }

	// totalDuration = time.Since(totalStart)
	// fmt.Printf("Total Execution Time in go for 100 screenshots : %.6f seconds\n", totalDuration.Seconds())

}
