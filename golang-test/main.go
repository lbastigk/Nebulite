package main

import (
	"bufio"
	"fmt"
	"io"
	"os/exec"
)

func main() {
	cmd := exec.Command("stdbuf", "-oL", "-eL", "./bin/Nebulite", "always dump-view")

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		fmt.Println("Error getting stdout:", err)
		return
	}

	stderr, err := cmd.StderrPipe()
	if err != nil {
		fmt.Println("Error getting stderr:", err)
		return
	}

	if err := cmd.Start(); err != nil {
		fmt.Println("Error starting process:", err)
		return
	}

	// Function now accepts io.Reader
	readPipe := func(prefix string, pipe io.Reader) {
		scanner := bufio.NewScanner(pipe)

		BufferSizeMiB := 20 // 20 MiB buffer
		buf := make([]byte, 0, BufferSizeMiB*1024*1024)
		scanner.Buffer(buf, 1024*1024)

		for scanner.Scan() {
			line := scanner.Text()

			if len(line) > 50 {
				line = line[:50]
			}

			fmt.Printf("[%s] %s\n", prefix, line)
		}

		if err := scanner.Err(); err != nil {
			fmt.Println("Scanner error:", err)
		}
	}

	go readPipe("COUT", stdout)
	go readPipe("CERR", stderr)

	err = cmd.Wait()
	if err != nil {
		fmt.Println("Process exited with error:", err)
	} else {
		fmt.Println("Process exited normally")
	}
}
