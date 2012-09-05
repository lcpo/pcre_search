#!/bin/bash

if [ ! "$1" ]; then
  echo "Usage: $0 [query]"
  exit 1
fi

echo "searching for: $1"

../curl_search "http://ajax.googleapis.com/ajax/services/search/web?v=1.0&q=$1" "titleNoFormatting\":\"(?P<title>[^\"]+)\"" title
