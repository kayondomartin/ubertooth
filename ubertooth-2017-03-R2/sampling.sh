#!/bin/bash

ubertooth-btle -R -U 1 >& log1  & ubertooth-btle -R -U 0 >& log0
