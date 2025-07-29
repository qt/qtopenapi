#!/bin/bash

set -e

# Clean up old binary if exists
rm -f backport-server-app

if [ -f "go.mod" ]; then
    go mod tidy
fi
#go mod download golang.org/x/sys
# Build the Go server
go build -o backport-server-app main.go

# Run the server in the background
./backport-server-app &

echo "Go server started in background with PID $!"
