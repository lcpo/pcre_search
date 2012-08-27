#!/bin/bash

./curl_search "http://www.archlinux.org/feeds/packages/" "<title>(?P<title>[^<]+)</title>" title
