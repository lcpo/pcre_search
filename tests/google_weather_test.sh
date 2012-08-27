#!/bin/bash

./curl_search "http://www.google.com/search?q=weather+21044" "<td\srowspan=\"2\"\sstyle=\"font-size:140%;white-space:nowrap;vertical-align:top;padding-right:15px;font-weight:bold\">(?P<temp>[^<]+)</td>" temp
