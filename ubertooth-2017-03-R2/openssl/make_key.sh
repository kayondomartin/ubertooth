#!/bin/bash

openssl genrsa -out private.pem 256
openssl rsa -in private.pem -out public.pem -outform PEM -pubout

./key_extract.sh public.pem public_key

