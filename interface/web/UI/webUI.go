package webUI

import (
	"bytes"
	"fmt"
	"image"
	"image/jpeg"
	"net/http"
	"sync"

	comm "web/communication"
)

type WebUI struct {
	comm       *comm.Communication
	server     *http.Server
	mu         sync.RWMutex
	currentImg image.Image
}

// NewWebUI creates a new web UI server
func NewWebUI(c *comm.Communication, port int) *WebUI {
	web := &WebUI{
		comm: c,
	}

	mux := http.NewServeMux()
	mux.HandleFunc("/", web.handleIndex)
	mux.HandleFunc("/frame.jpg", web.handleFrameJPEG)
	mux.HandleFunc("/api/frame", web.handleFrameAPI)

	web.server = &http.Server{
		Addr:    fmt.Sprintf(":%d", port),
		Handler: mux,
	}

	return web
}

// Start starts the web server in a goroutine
func (w *WebUI) Start() error {
	go func() {
		if err := w.server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			fmt.Printf("Web server error: %v\n", err)
		}
	}()
	return nil
}

// Stop gracefully stops the web server
func (w *WebUI) Stop() error {
	if w.server != nil {
		return w.server.Close()
	}
	return nil
}

// UpdateFrame updates the current frame from communication
func (w *WebUI) UpdateFrame() {
	view := w.comm.LastView()
	if view.Data == nil || len(view.Data) == 0 {
		return
	}

	// Decode the raw frame data
	img, err := decodeFrameData(&view)
	if err != nil {
		fmt.Printf("Error decoding frame: %v\n", err)
		return
	}

	w.mu.Lock()
	w.currentImg = img
	w.mu.Unlock()
}

// handleIndex serves a simple HTML page
func (w *WebUI) handleIndex(wr http.ResponseWriter, r *http.Request) {
	wr.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprint(wr, `
<!DOCTYPE html>
<html>
<head>
	<title>Nebulite Frame Viewer</title>
	<style>
		body {
			font-family: Arial, sans-serif;
			max-width: 1200px;
			margin: 0 auto;
			padding: 20px;
			background: #f0f0f0;
		}
		.container {
			background: white;
			padding: 20px;
			border-radius: 8px;
			box-shadow: 0 2px 4px rgba(0,0,0,0.1);
		}
		h1 {
			color: #333;
		}
		#frameContainer {
			text-align: center;
			margin: 20px 0;
		}
		#frame {
			max-width: 100%;
			max-height: 600px;
			border: 1px solid #ddd;
			border-radius: 4px;
		}
		.info {
			background: #f9f9f9;
			padding: 10px;
			margin: 10px 0;
			border-left: 4px solid #007bff;
		}
	</style>
	<script>
		// Refresh frame every 100ms
		setInterval(function() {
			const img = document.getElementById('frame');
			img.src = '/frame.jpg?' + Date.now();
		}, 100);
	</script>
</head>
<body>
	<div class="container">
		<h1>Nebulite Frame Viewer</h1>
		<div class="info">
			<p>Current frame from Nebulite simulation</p>
		</div>
		<div id="frameContainer">
			<img id="frame" src="/frame.jpg" alt="Current frame"/>
		</div>
	</div>
</body>
</html>
	`)
}

// handleFrameJPEG serves the current frame as JPEG
func (w *WebUI) handleFrameJPEG(wr http.ResponseWriter, r *http.Request) {
	w.mu.RLock()
	img := w.currentImg
	w.mu.RUnlock()

	if img == nil {
		wr.WriteHeader(http.StatusNotFound)
		fmt.Fprint(wr, "No frame available")
		return
	}

	wr.Header().Set("Content-Type", "image/jpeg")
	wr.Header().Set("Cache-Control", "no-cache")

	if err := jpeg.Encode(wr, img, &jpeg.Options{Quality: 85}); err != nil {
		fmt.Printf("Error encoding JPEG: %v\n", err)
	}
}

// handleFrameAPI serves frame info as JSON
func (w *WebUI) handleFrameAPI(wr http.ResponseWriter, r *http.Request) {
	view := w.comm.LastView()
	wr.Header().Set("Content-Type", "application/json")
	fmt.Fprintf(wr, `{"type":"%s","width":%d,"height":%d,"format":"%s"}`,
		view.Type, view.Width, view.Height, view.Format)
}

// decodeFrameData decodes frame data (JPEG) to an image
func decodeFrameData(view *comm.FrameView) (image.Image, error) {
	if view.Data == nil || len(view.Data) == 0 {
		return nil, fmt.Errorf("no frame data available")
	}

	// Data is already JPEG bytes (auto-decoded from base64 by JSON unmarshaler)
	img, err := jpeg.Decode(bytes.NewReader(view.Data))
	if err != nil {
		return nil, fmt.Errorf("failed to decode JPEG: %w", err)
	}

	return img, nil
}
