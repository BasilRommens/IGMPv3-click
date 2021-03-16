#!/bin/bash

echo "write home_mobility.switch -1" | telnet localhost 10000
echo "write mobility.switch 1" | telnet localhost 10000
echo "write foreign_mobility.switch 0" | telnet localhost 10000
