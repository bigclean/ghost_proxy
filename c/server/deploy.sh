#!/bin/sh

ghost_server_src=/Users/bigclean/develop/ghost_proxy/c/server/
ghost_servere_dst=higurashi:/home/bigclean/programming/ghost_proxy/c/server/

sync_dryrun()
{
    echo
    echo -e "\033[1m ...dryrun...\033[0m"
    rsync -vr --delete $tcl_active_src $tcl_active_dst --dry-run
    echo -e "\033[1m ...dryrun...\033[0m"
    echo
}

sync_server()
{
    echo
    echo -e "\033[1m ...syncing...\033[0m"
    rsync -arv --progress --delete $ghost_server_src $ghost_servere_dst
    echo -e "\033[1m ...done...\033[0m"
    echo
}
sync_server
