#!/bin/bash

#case 2 multiple swaps
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=2 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.92 --nSwap=2 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.91 --nSwap=2 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.9  --nSwap=2 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.88 --nSwap=2 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.85 --nSwap=2 --policy=DBDP"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=3 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.92 --nSwap=3 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.91 --nSwap=3 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.9  --nSwap=3 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.88 --nSwap=3 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.85 --nSwap=3 --policy=DBDP"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=4 --policy=DBDP"
#time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.92 --nSwap=4 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.91 --nSwap=4 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.9  --nSwap=4 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.88 --nSwap=4 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.85 --nSwap=4 --policy=DBDP"


