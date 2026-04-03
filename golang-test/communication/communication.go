package communication

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"os/exec"
	"strings"
	"sync"
)

type lineEvent struct {
	prefix string
	line   string
}

type FrameView struct {
	Type     string `json:"type"`
	Encoding string `json:"encoding"`
	Width    int    `json:"width"`
	Height   int    `json:"height"`
	Format   string `json:"format"`
	Data     []byte `json:"data"`
}

type Communication struct {
	cmd   *exec.Cmd
	lines chan lineEvent

	mu       sync.RWMutex
	active   bool
	lastLine string
	lastView FrameView
	err      error
}

func NewCommunication() (*Communication, error) {
	cmd := exec.Command(
		"stdbuf",
		"-oL",
		"-eL",
		"./bin/Nebulite",
		"always dump-view ;",
		"task TaskFiles/Benchmarks/gravity_XL.nebs",
	)
	return NewCommunicationFromCmd(cmd)
}

func NewCommunicationFromCmd(cmd *exec.Cmd) (*Communication, error) {
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return nil, fmt.Errorf("stdout pipe: %w", err)
	}

	stderr, err := cmd.StderrPipe()
	if err != nil {
		return nil, fmt.Errorf("stderr pipe: %w", err)
	}

	comm := &Communication{
		cmd:    cmd,
		lines:  make(chan lineEvent, 256),
		active: true,
	}

	if err := cmd.Start(); err != nil {
		return nil, fmt.Errorf("start process: %w", err)
	}

	var readers sync.WaitGroup
	readers.Add(2)

	go comm.readPipe("COUT", stdout, &readers)
	go comm.readPipe("CERR", stderr, &readers)

	go func() {
		waitErr := cmd.Wait()
		readers.Wait()
		close(comm.lines)

		comm.mu.Lock()
		comm.active = false
		comm.err = waitErr
		comm.mu.Unlock()
	}()

	return comm, nil
}

func (c *Communication) readPipe(prefix string, pipe io.Reader, wg *sync.WaitGroup) {
	defer wg.Done()

	scanner := bufio.NewScanner(pipe)

	const bufferSizeMiB = 20
	buf := make([]byte, 0, bufferSizeMiB*1024*1024)
	scanner.Buffer(buf, bufferSizeMiB*1024*1024)

	for scanner.Scan() {
		line := scanner.Text()
		c.lines <- lineEvent{prefix: prefix, line: line}

		// Try to parse JSON
		var tmp FrameView
		if err := json.Unmarshal([]byte(line), &tmp); err == nil {
			// Success → update lastView
			c.lastView = tmp
		}
	}

	if err := scanner.Err(); err != nil {
		c.lines <- lineEvent{prefix: prefix, line: "scanner error: " + err.Error()}
	}
}

func (c *Communication) Update() {
	for {
		select {
		case event, ok := <-c.lines:
			if !ok {
				return
			}

			formatted := fmt.Sprintf("[%s] %s", event.prefix, strings.TrimSpace(event.line))
			c.mu.Lock()
			c.lastLine = formatted
			c.mu.Unlock()
		default:
			return
		}
	}
}

func (c *Communication) Active() bool {
	c.mu.RLock()
	active := c.active
	c.mu.RUnlock()

	if active {
		return true
	}

	return len(c.lines) > 0
}

func (c *Communication) LastLine() string {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.lastLine
}

func (c *Communication) LastView() FrameView {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.lastView
}

func (c *Communication) Err() error {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.err
}
