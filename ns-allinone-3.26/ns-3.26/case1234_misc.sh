#!/bin/bash
#case 1
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.6  --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.59 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.58 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.56 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.55 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.52 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.5 --nSwap=1 --policy=DBDP"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.62 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.61 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.6  --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.58 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.55 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.5  --policy=LDF"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.45 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.44 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.42 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.4  --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.35 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=1 --alpha=0.3  --policy=FCSMA"

#case 2
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.92 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.91 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.9  --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.88 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.85 --nSwap=1 --policy=DBDP"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.95 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.94 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.93 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.92 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.90 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.85 --nSwap=1 --policy=LDF"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.83 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.82 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.81 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.8  --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.78 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.76 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=2 --alpha=0.55 --q=0.75 --nSwap=1 --policy=FCSMA"

#case 3
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.75 --q=0.9 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.74 --q=0.9 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.73 --q=0.9 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.72 --q=0.9 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.7  --q=0.9 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.65 --q=0.9 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.6  --q=0.9 --nSwap=1 --policy=DBDP"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.76 --q=0.9 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.75 --q=0.9 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.74 --q=0.9 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.72 --q=0.9 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.7  --q=0.9 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.65 --q=0.9 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.6  --q=0.9 --nSwap=1 --policy=LDF"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.52 --q=0.9 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.51 --q=0.9 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.5  --q=0.9  --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.48 --q=0.9 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.45 --q=0.9 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.42 --q=0.9 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=3 --alpha=0.4  --q=0.9 --nSwap=1 --policy=FCSMA"

#case 4
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.93 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.92 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.91 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.9  --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.88 --nSwap=1 --policy=DBDP"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.85 --nSwap=1 --policy=DBDP"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.94 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.93 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.92 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.90 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.88 --nSwap=1 --policy=LDF"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.85 --nSwap=1 --policy=LDF"

time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.8  --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.79 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.78 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.76 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.74 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.72 --nSwap=1 --policy=FCSMA"
time ./waf --run "scratch/RT-decentralized --nIntervals=5000 --testId=4 --alpha=0.7 --q=0.7  --nSwap=1 --policy=FCSMA"



