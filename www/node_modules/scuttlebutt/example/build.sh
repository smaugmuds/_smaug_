#!/usr/bin/env bash
{
  echo '<!DOCTYPE HTML><html><body></body><script>'
  browserify index.js --debug || exit 1
  echo '</script></html>'
} > index.html

