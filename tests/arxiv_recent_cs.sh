#!/bin/bash

#valgrind --leak-check=full ../curl_search "http://export.arxiv.org/rss/cs" "<item rdf:about[^>]+>.*?<title>(?P<title>[^<]+)</title>.*?<link>(?P<link>[^<]+)</link>.*?</item>"
../curl_search "http://export.arxiv.org/rss/cs" "<item rdf:about[^>]+>.*?<title>(?P<title>[^<]+)</title>.*?<link>(?P<link>[^<]+)</link>.*?</item>"
