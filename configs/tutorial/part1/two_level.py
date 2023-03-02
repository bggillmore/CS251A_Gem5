#import m5
from m5.objects import *
from cache import *
# import all of the SimObjects

#from gem5.runtime import get_runtime_isa

import argparse

parser = argparse.ArgumentParser(description='A simple system with 2-level cache.')
parser.add_argument("binary", default="", nargs="?", type=str,
                    help="Path to the binary to execute.")
parser.add_argument("--l1i_size",
                    help=f"L1 instruction cache size. Default: 16kB.")
parser.add_argument("--l1d_size",
                    help="L1 data cache size. Default: Default: 64kB.")
parser.add_argument("--l2_size",
                    help="L2 cache size. Default: 256kB.")

options = parser.parse_args()

# System object - parent of all other objects in simulated system
system = System()

system.workload = SEWorkload.init_compatible(options.binary)

# Set clock domain, freq, and voltage domain for clock domain
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()

# Set timing mode for memory simulation
#   - You will almost always use timing mode
system.mem_mode = 'timing'
# Single memory range of 512MB
system.mem_ranges = [AddrRange('512MB')]

# Create CPU (X86 ISA)
system.cpu = X86TimingSimpleCPU()

# Create L1 caches
system.cpu.icache = L1ICache(options)
system.cpu.dcache = L1DCache(options)

# Connect caches to cpu ports
system.cpu.icache.connectCPU(system.cpu)
system.cpu.dcache.connectCPU(system.cpu)

# Create a memory bus to connect our L1 caches to your L2 cache
system.l2bus = L2XBar()

# Connect L1 cache to memory bus
system.cpu.icache.connectBus(system.l2bus)
system.cpu.dcache.connectBus(system.l2bus)

# Create L2 Chache
system.l2cache = L2Cache(options)

# Connect L2 cahce to memory bus
system.l2cache.connectCPUSideBus(system.l2bus)

# Create system wide memory bus
system.membus = SystemXBar()

# Connect L2 cache to main memory bus
system.l2cache.connectMemSideBus(system.membus)

# Connect the cache to the memory bus
#system.cpu.icache_port = system.membus.cpu_side_ports
#system.cpu.dcache_port = system.membus.cpu_side_ports

# Create I/O controller on CPU and connect to memory bus
# This is X86 specific requiremnt and not needed on other ISAs
system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.mem_side_ports

# Connect interupt controller to memory bus
system.cpu.interrupts[0].int_requestor = system.membus.cpu_side_ports
system.cpu.interrupts[0].int_responder = system.membus.mem_side_ports

# Connect special (functional) port that allows the system to r/w memory
system.system_port = system.membus.cpu_side_ports

# Create memory controller and connect it to memory bus (ddr3 in this case)
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

# Next we need to set up the process we want the CPU to execute
binary = 'tests/test-progs/hello/bin/x86/linux/hello'

# Create and set processes command to command we want to run
process = Process()

# This is similar to argv
#   - executable in first position and arguements as the rest of the list
process.cmd = [binary]

# Set spu to use process as its workload
system.cpu.workload = process

# Create functional execution context
system.cpu.createThreads()

system.workload = SEWorkload.init_compatible(options.binary)
# Instantiate the system
root = Root(full_system = False, system = system)
m5.instantiate()

# Begin execution
print("Beginning simulation!")
exit_event = m5.simulate()

# When system finishes inspect the state of the system
print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))

