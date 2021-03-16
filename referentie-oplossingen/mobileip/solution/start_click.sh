#!/bin/sh

cd /home/student/click/scripts

/home/student/click/userlevel/click /home/student/click/scripts/glue.click -p 10000 &

sleep 1

# /home/student/click/userlevel/click /home/student/click/scripts/mn.click -p 10001 &
# /home/student/click/userlevel/click /home/student/click/scripts/ha.click -p 10002 &
# /home/student/click/userlevel/click /home/student/click/scripts/fa.click -p 10003 &
/home/student/click-reference/solution/mn.bin &
/home/student/click-reference/solution/ha.bin &
/home/student/click-reference/solution/fa.bin &
/home/student/click/userlevel/click /home/student/click/scripts/cn.click -p 10004

wait

