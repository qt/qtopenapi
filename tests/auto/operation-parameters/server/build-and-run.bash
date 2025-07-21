#!/bin/bash

set -e

# Clean up old binary if exists
rm -f server-app

# Build the Go server
go build -o server-app main.go

# Run the server in the background
./server-app &

echo "Go server started in background with PID $!"
