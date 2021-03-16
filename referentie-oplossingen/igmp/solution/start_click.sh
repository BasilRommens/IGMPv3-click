#!/bin/sh

cd /home/student/click-reference/solution/

/home/student/click/userlevel/click /home/student/click/scripts/glue.click &

sleep 1

./router.bin &   # port 10001
./client21.bin & # port 10003
./client22.bin & # port 10004
./client31.bin & # port 10005
./client32.bin & # port 10006
./server.bin &   # port 10002

wait

