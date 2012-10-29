#!/bin/bash

#../curl_search "http://news.ycombinator.com" "<td\sclass=\"title\"><a[^>]+>\s?(?P<title>[^<]+)</a>"


../curl_search "http://news.ycombinator.com" "<td\sclass=\"title\"><a\shref=\"(?P<link>[^>]+)\">\s?(?P<title>[^<]+)</a>"
