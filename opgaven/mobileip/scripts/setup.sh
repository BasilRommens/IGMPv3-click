#!/bin/bash

for i in ` seq 0 5`;
do
	tunctl && ip link set dev tap$i up
done

