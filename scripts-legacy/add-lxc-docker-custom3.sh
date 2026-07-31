#!/bin/bash
CONFIGF=$1
cd $GITHUB_WORKSPACE/kernel_workspace
wget https://github.com/ego-taboo/LXC-DOCKER-KernelSU_Action/releases/download/2.0.0/lxc-docker-config3.txt
aa=$(cat lxc-docker-config3.txt)
for i in $aa
do
echo $i >> $CONFIGF
done
