#!/bin/bash

../curl_search "http://news.ycombinator.com" "<td\sclass=\"title\"><a[^>]+>\s?(?P<title>[^<]+)</a>" title
