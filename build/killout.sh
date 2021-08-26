#!/bin/bash
xxx=$(ps aux |grep ./meshserver.out |awk '{print $2}')
echo $xxx
kill -9 $xxx
