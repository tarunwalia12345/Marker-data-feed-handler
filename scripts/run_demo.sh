#!/bin/bash

echo "Running demo..."

# start server in background
./scripts/run_server.sh &
SERVER_PID=$!

sleep 1

# run client
./scripts/run_client.sh

# stop server after client exits
kill $SERVER_PID

echo "Demo finished."