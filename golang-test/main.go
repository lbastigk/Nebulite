package main

import (
	"bufio"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"image"
	"image/png"
	"io"
	"log"
	"os"
	"os/exec"
	"strings"
)

func main() {
	cmd := exec.Command("./bin/Nebulite", "--headless", "task", "TaskFiles/Debugging/pixelView.nebs")
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Fatal(err)
	}
	stderr, err := cmd.StderrPipe()
	if err != nil {
		log.Fatal(err)
	}

	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}

	reader := bufio.NewReader(io.MultiReader(stdout, stderr))
	var latestFrame map[string]interface{}
	frameCount := 0

	for {
		line, err := reader.ReadString('\n')
		if err != nil {
			if err == io.EOF {
				break
			}
			log.Fatal("Read error:", err)
		}

		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}

		// Debug: first 100 characters
		fmt.Println("DEBUG LINE:", line[:min(len(line), 100)])

		if strings.HasPrefix(line, "{") {
			var obj map[string]interface{}
			if err := json.Unmarshal([]byte(line), &obj); err != nil {
				fmt.Println("Failed to decode JSON:", err)
				continue
			}

			// Debug keys
			fmt.Printf("DEBUG: Read JSON keys: %v\n", keys(obj))

			if t, ok := obj["type"]; ok && t == "frame" {
				format, ok := obj["format"].(string)
				if !ok || format != "rgba" {
					log.Fatalf("Unsupported frame format: %v", obj["format"])
				}

				latestFrame = obj
				frameCount++
				fmt.Println("DEBUG: Latest frame updated, width/height/format:",
					obj["width"], obj["height"], obj["format"])
			}
		}
	}

	fmt.Printf("Total frames found: %d\n", frameCount)
	if latestFrame != nil {
		encoded, _ := latestFrame["data"].(string)
		width := int(latestFrame["width"].(float64))
		height := int(latestFrame["height"].(float64))

		rawData, err := base64.StdEncoding.DecodeString(encoded)
		if err != nil {
			log.Fatal("Base64 decode failed:", err)
		}

		// Convert raw RGBA bytes to image.RGBA
		img := image.NewRGBA(image.Rect(0, 0, width, height))
		copy(img.Pix, rawData)

		// Save as PNG
		fileName := "latest_frame.png"
		outFile, err := os.Create(fileName)
		if err != nil {
			log.Fatal("Failed to create PNG file:", err)
		}
		defer outFile.Close()

		if err := png.Encode(outFile, img); err != nil {
			log.Fatal("Failed to encode PNG:", err)
		}

		fmt.Println("Frame saved as PNG to", fileName)
	} else {
		fmt.Println("No frame found in output.")
	}
}

func keys(m map[string]interface{}) []string {
	ks := make([]string, 0, len(m))
	for k := range m {
		ks = append(ks, k)
	}
	return ks
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}
