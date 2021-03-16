#!/bin/sh


printf "\e[31m\n\nWARNING\nThis will remove all data! Press Ctrl-C to abort, or Enter to continue\e[0m\n\n\n"
read x

set -uex

rm -rf $HOME/click/scripts/ $HOME/click/elements/local/{rsvp,igmp,mobileip}
rm -rf $HOME/click-reference/

mkdir -p $HOME/click/elements/local/{rsvp,igmp,mobileip}

