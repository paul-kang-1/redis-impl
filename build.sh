#!/usr/bin/env bash

# TODO: add cmake config
rm -rf ./bin/client ./bin/server
g++ -Wall -Wextra -O3 client.cpp -o ./bin/client
g++ -Wall -Wextra -O3 server.cpp -o ./bin/server
