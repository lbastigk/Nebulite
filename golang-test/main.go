package main

import (
	"fmt"
	"time"

	comm "golang-test/communication"
	webUI "golang-test/web"
)

func main() {
	communication, err := comm.NewCommunication()
	if err != nil {
		fmt.Println("Error creating communication:", err)
		return
	}

	// Create and start the web UI server
	web := webUI.NewWebUI(communication, 8080)
	if err := web.Start(); err != nil {
		fmt.Println("Error starting web server:", err)
		return
	}
	defer web.Stop()

	fmt.Println("Web UI server started at http://localhost:8080")

	var previous string
	for communication.Active() {
		communication.Update()

		line := communication.LastLine()
		if line != "" && line != previous {
			previous = line

			// Show info about last view:
			view := communication.LastView()
			if view.Width > 0 && view.Height > 0 {
				web.UpdateFrame()
				// TODO: Process any key/mouse events to later send back to Nebulite
				//       Must be interpreted as SDL_event in renderer function, so that imgui can read them
				//       Idea: renderer event override DomainModule, pushes events for mouse/keyboard into event vector
				//       But first, we need an inter-process communication bus
			}
		}

		// Avoid busy-waiting while still polling for new lines.
		const maxFps = 60
		const msDelay = 1000 / maxFps
		time.Sleep(msDelay * time.Millisecond)
	}

	if err := communication.Err(); err != nil {
		fmt.Println("Process exited with error:", err)
	} else {
		fmt.Println("Process exited normally")
	}
}
