#!/bin/bash
tx_level=$1

sudo ubertooth-btle -s ec:55:f9:12:7c:c9 -l $tx_level -U 0
