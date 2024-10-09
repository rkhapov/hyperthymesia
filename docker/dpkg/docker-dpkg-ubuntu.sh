#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Expected arg: codename of ubuntu (bionic, jammy, etc)"
    exit 1
fi

if [ -z "$2" ]; then
    echo "Expected arg: output path"
    exit 1
fi

set -eux

docker build . --tag hyperthymesia_dpkg -f ./docker/dpkg/Dockerfile.ubuntu --build-arg codename="$1"
cid=`docker create hyperthymesia_dpkg:latest`
docker cp ${cid}:/hyperthymesia/_packages "$2"
docker rm ${cid}
