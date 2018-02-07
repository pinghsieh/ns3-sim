#!/bin/bash

# case 5
#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=5 --lambda=0.6 --q=0.99 --nSwap=1 --policy=LDF"
#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=5 --lambda=0.5 --q=0.99 --nSwap=1 --policy=LDF"
#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=5 --lambda=0.4 --q=0.99 --nSwap=1 --policy=LDF"

#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=5 --lambda=0.5 --q=0.99 --nSwap=1 --policy=DBDP"
#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=5 --lambda=0.4 --q=0.99 --nSwap=1 --policy=DBDP"

#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=5 --lambda=0.34 --q=0.99 --nSwap=1 --policy=FCSMA"
#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=5 --lambda=0.28 --q=0.99 --nSwap=1 --policy=FCSMA"

# case 6
#time ./waf --run "scratch/RT-decentralized --nIntervals=20000 --testId=6 --lambda=0.78 --q=0.992 --nSwap=1 --policy=DBDP"


#case 2 multiple swaps
#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.94 --nSwap=2 --policy=DBDP"
#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.94 --nSwap=3 --policy=DBDP"
#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.94 --nSwap=4 --policy=DBDP"

#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=1 --policy=DBDP"
#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=2 --policy=DBDP"
#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=3 --policy=DBDP"
#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=4 --policy=DBDP"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.92 --nSwap=4 --policy=DBDP"

# case 3
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.71 --q=0.9 --nSwap=1 --policy=DBDP"

# case 4
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.9  --nSwap=1 --policy=DBDP"
