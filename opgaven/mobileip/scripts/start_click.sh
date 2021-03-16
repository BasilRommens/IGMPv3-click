#!/bin/bash

cd /home/student/click/scripts

../userlevel/click glue.click -p 10000 &
../userlevel/click mn.click -p 10001 &
../userlevel/click ha.click -p 10002 &
../userlevel/click fa.click -p 10003 &
../userlevel/click cn.click -p 10004

