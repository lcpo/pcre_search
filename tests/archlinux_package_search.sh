#!/bin/bash

if [ ! "$1" ]; then
  echo "Usage: $0 [query]"
  exit 1
fi

echo "searching for: $1"

../curl_search "https://www.archlinux.org/packages/?q=$1" \
               "<a\s[^>]+?title=\"View package details for[^>]+?>(?P<title>[a-zA-Z\-\_]+)</a></td><td>[^<]+?</td><td\sclass=\"wrap\">(?P<description>[^<]+)</td>"
