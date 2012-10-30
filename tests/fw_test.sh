#!/bin/bash

if [ ! "$1" ]; then
  echo "Usage: $0 [query]"
  exit 1
fi

../curl_search "http://thefuckingweather.com/?where=$1" \
               "<span class=\"temperature\"[^>]+>(?P<temp>[^<]+)<\/span>.*?<p class=\"remark\">(?P<remark>[^<]+)<\/p>.*?<p class=\"flavor\">(?P<flavortext>[^<]+)<\/p>"
