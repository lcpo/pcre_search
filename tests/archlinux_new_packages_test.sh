#!/bin/bash

#../curl_search "https://www.archlinux.org/feeds/packages/" "<title>(?P<title>[^<]+)</title>"


../curl_search "https://www.archlinux.org/feeds/packages/" "<item><title>(?P<title>[^<]+)</title><link>[^<]+</link><description>(?P<description>[^<]+)</description>.*?</item>"
