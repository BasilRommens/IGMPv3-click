#!/bin/sh

cd "$(dirname "$0")"

set -uex
if ./clean.sh; then
	cp -r $HOME/opgaven/mobileip/scripts $HOME/click/scripts
	cp -r $HOME/referentie-oplossingen/mobileip $HOME/click-reference
fi


