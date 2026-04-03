package main

import (
	"fmt"
	"time"

	comm "golang-test/communication"
)

func main() {
	communication, err := comm.NewCommunication()
	if err != nil {
		fmt.Println("Error creating communication:", err)
		return
	}

	var previous string
	for communication.Active() {
		communication.Update()

		line := communication.LastLine()
		if line != "" && line != previous {
			if len(line) > 50 {
				fmt.Println(line[:50])
			} else {
				fmt.Println(line)
			}
			previous = line

			// Show info about last view:
			view := communication.LastView()
			fmt.Println(view)

			// TODO: Convert to jpeg and show
		}

		// Avoid busy-waiting while still polling for new lines.
		time.Sleep(10 * time.Millisecond)
	}

	if err := communication.Err(); err != nil {
		fmt.Println("Process exited with error:", err)
	} else {
		fmt.Println("Process exited normally")
	}
}
