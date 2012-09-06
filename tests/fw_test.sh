#!/bin/bash

if [ ! "$1" ]; then
  echo "Usage: $0 [query]"
  exit 1
fi

../curl_search "http://thefuckingweather.com/?where=$1" "<p\sclass=\"flavor\">(?P<flavortext>[^<]+?)</p>" flavortext
