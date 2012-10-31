#!/bin/bash

../curl_search "https://api.github.com/repos/rapid7/metasploit-framework/commits" '\"message\":\s*\"(?P<msg>[^\\\"]+)'
