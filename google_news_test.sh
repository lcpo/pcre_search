#!/bin/bash

./curl_search "http://ajax.googleapis.com/ajax/services/search/news?v=1.0&topic=snc" "titleNoFormatting\":\"(?P<title>.*?)\"" title
