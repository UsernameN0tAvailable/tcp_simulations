#!/usr/bin/env bash



ruby simulate.rb 0.001 4 5.5 & ruby simulate.rb 0.001 4 6.5 & ruby simulate.rb 0.001 4 8 &
wait
