#!/bin/bash

if [ ! "$1" ]; then
  echo "Usage: $0 [query]"
  exit 1
fi

../curl_search "http://www.google.com/search?q=weather+$1" "<td\srowspan=\"2\"\sstyle=\"font-size:140%;white-space:nowrap;vertical-align:top;padding-right:15px;font-weight:bold\">(&[^;]+?;)*(?P<temp>[^<]+)</td>"
