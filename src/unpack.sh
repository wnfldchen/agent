#!/usr/bin/env bash
cd $(dirname "$0")
xxd -i unpack > ../include/unpack.h
