#!/bin/bash

echo "🚀 Running demo..."

./scripts/run_server.sh &
SERVER_PID=$!

sleep 2

./scripts/run_client.sh

# Kill server after client exits
kill $SERVER_PID