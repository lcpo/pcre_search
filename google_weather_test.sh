#!/bin/bash

./curl_search http://www.google.com/ig/api?weather=21044 "<current_conditions><condition\sdata=\"(?P<condition>[a-zA-Z\s]+)\"/><temp_f\sdata=\"(?P<temp>\d+)\"/>" temp 
