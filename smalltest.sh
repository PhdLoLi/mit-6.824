killall lock_server
export RPC_LOSSY=5
./lock_server 3772 &
./lock_tester 3772
export RPC_LOSSY=0
killall lock_server
