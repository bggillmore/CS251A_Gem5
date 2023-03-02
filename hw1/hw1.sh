#!/bin/bash
path=cs251a-microbench
benchmark=(mm spmv lfsr merge sieve)

rm hw1_results/*

function run(){
	pushd cs251a-microbench
	make clean
	if [ $opt == -O3 ]
	then
		make OPT=${opt} CFLAGS="-ffast-math -ftree-vectorize -Wall -Wextra -Wpedantic -Wstrict-aliasing -static"
	else
		make OPT=${opt} CFLAGS="-Wall -Wextra -Wpedantic -Wstrict-aliasing -static"
	fi
	popd
	for i in "${benchmark[@]}" 
	do
		cmd="build/X86/gem5.opt configs/example/se.py --cmd=${path}/${i} --cpu-type=${cpu_type} --cpu-clock=${cpu_clock} --issueWidth=${issue_width} --mem-type=${mem_type} --l1d_size=${l1d_size} --l1i_size=${l1i_size}"
		if [ $l2_cache_en == 1 ] 
		then 
			cmd+=" --l2_size=${l2_size}"
		fi 
		cmd+=" --caches"
		${cmd}

		cp m5out/config.ini hw1_results/${cpu_type}_${cpu_clk}_issueWidth_${issue_width}_l2Cache_${l2_cache_en}_${l2_size}_opt_${opt#*-}__${i}_config.ini
		cp m5out/stats.txt hw1_results/${cpu_type}_${cpu_clk}_issueWidth_${issue_width}_l2Cache_${l2_cache_en}_${l2_size}_opt_${opt#*-}_${i}_stats.txt 
	done
}


cpu_type=DerivO3CPU 
cpu_clock=1GHz 
issue_width=8
mem_type=DDR3_1600_8x8 
l1d_size=64kB 
l1i_size=16kB 
l2_size=2MB 
opt=-O3
l2_cache_en=1
run

cpu_type=X86MinorCPU
run

cpu_type=DerivO3CPU 
issue_width=2
run

issue_width=8
cpu_clock=4GHz
run

cpu_clock=1GHz
#No L2 Cache, 256KB L2, 2MB L2(already done), 16MB L2
l2_cache_en=0
l2_size=0MB
run

l2_cache_en=1
l2_size=256kB
run

l2_size=16MB
run

l2_size=2MB
opt=-O1
run
