#!/usr/bin/env sh

DIRECTORY=$(cd `dirname $0` && pwd)

"$DIRECTORY/yaml2json" < "$DIRECTORY/../resources/spec.yml" > "$DIRECTORY/../resources/document_root/api/v1/spec.json"