#!/bin/bash

../curl_search "http://news.ycombinator.com" "<td\sclass=\"title\"><a[^>]+>(?P<title>[^<]+)</a>" title
