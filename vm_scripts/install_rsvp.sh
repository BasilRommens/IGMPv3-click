#!/bin/sh

cd "$(dirname "$0")"

set -uex
if ./clean.sh; then
	cp -r $HOME/opgaven/rsvp/scripts $HOME/click/scripts
	cp -r $HOME/referentie-oplossingen/rsvp $HOME/click-reference
fi

