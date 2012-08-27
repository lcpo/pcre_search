#!/bin/bash

../curl_search "http://ajax.googleapis.com/ajax/services/search/web?v=1.0&q=archlinux" "titleNoFormatting\":\"(?P<title>[^\"]+)\"" title
