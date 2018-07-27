#!/bin/bash
u=$1

sudo ubertooth-btle -f -U $1 >& log$u
