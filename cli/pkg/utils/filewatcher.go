package utils

import (
	"os"
	"path/filepath"
	"sync"
	"time"

	"github.com/fsnotify/fsnotify"
)

type FileWatcher struct {
	watcher *fsnotify.Watcher
	change  chan bool
	quit    chan struct{}
	wg      sync.WaitGroup
}

func NewFileWatcher(root string, dirs []string) (*FileWatcher, error) {
	w, err := fsnotify.NewWatcher()
	if err != nil {
		return nil, err
	}
	fw := &FileWatcher{
		watcher: w,
		change:  make(chan bool, 1),
		quit:    make(chan struct{}),
	}
	for _, d := range dirs {
		absDir := filepath.Join(root, d)
		filepath.Walk(absDir, func(path string, info os.FileInfo, err error) error {
			if err == nil && info != nil && info.IsDir() {
				w.Add(path)
			}
			return nil
		})
	}
	fw.wg.Add(1)
	go fw.run()
	return fw, nil
}

func (fw *FileWatcher) run() {
	defer fw.wg.Done()
	debounce := time.Now()
	for {
		select {
		case event := <-fw.watcher.Events:
			if event.Op&(fsnotify.Write|fsnotify.Create|fsnotify.Remove|fsnotify.Rename) != 0 {
				if time.Since(debounce) > 300*time.Millisecond {
					fw.change <- true
					debounce = time.Now()
				}
			}
		case <-fw.quit:
			fw.watcher.Close()
			return
		}
	}
}

func (fw *FileWatcher) WaitForChange() bool {
	return <-fw.change
}

func (fw *FileWatcher) Close() {
	close(fw.quit)
	fw.wg.Wait()
}
