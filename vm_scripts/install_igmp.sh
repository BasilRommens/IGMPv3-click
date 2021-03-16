#!/bin/sh

cd "$(dirname "$0")"

set -uex
if ./clean.sh; then
	cp -r $HOME/opgaven/igmp/scripts $HOME/click/scripts
	cp -r $HOME/referentie-oplossingen/igmp $HOME/click-reference
fi

