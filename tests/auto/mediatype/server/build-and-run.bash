#!/bin/bash

set -e

# Clean up old binary if exists
rm -f mediatype-server-app

if [ -f "go.mod" ]; then
    go mod tidy
fi

# Build the Go server
go build -o mediatype-server-app main.go

# Run the server in the background
./mediatype-server-app &

echo "Go server started in background with PID $!"
