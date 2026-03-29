#!/bin/bash

echo "🚀 Running demo..."

# Run server in background
./scripts/run_server.sh &
SERVER_PID=$!

sleep 2

# Run client
./scripts/run_client.sh

# Kill server after client exits
kill $SERVER_PID