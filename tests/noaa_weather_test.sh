#!/bin/bash

if [ ! "$1" ]; then
  echo "Usage: $0 [query]"
  exit 1
fi

lat=$(../curl_search "http://maps.googleapis.com/maps/api/geocode/json?address=$1&sensor=false" "\"location\"\s:\s\{[^\}]+\"lat\"\s:\s(?P<lat>[^,]+)," lat | cut -d' ' -f5)
lng=$(../curl_search "http://maps.googleapis.com/maps/api/geocode/json?address=$1&sensor=false" "\"location\"\s:\s\{[^\}]+\"lng\"\s:\s(?P<lng>[^\n]+)\n" lng | cut -d' ' -f5)

echo "Lat: $lat"
echo "Lng: $lng"

../curl_search "http://forecast.weather.gov/MapClick.php?lat=$lat&lon=$lng" "<p\sclass=\"myforecast-current\">\s?(?P<current_conditions>[^<]+)</p>" current_conditions
