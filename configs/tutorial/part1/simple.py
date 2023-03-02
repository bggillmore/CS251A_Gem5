import m5
from m5.objects import *

# System object - parent of all other objects in simulated system
system = System()

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

# Create system wide memory bus
system.membus = SystemXBar()

# Connect the cahce to the memory bus
system.cpu.icache_port = system.membus.cpu_side_ports
system.cpu.dcache_port = system.membus.cpu_side_ports

# To connect memory components gem5 uses port abstraction
#   2 types of ports
#       - request ports
#       - response ports
#       - requests sent from request port -> response port
#       - responses sent from response port -> request port

# Connecting ports together is as easy as '='
# Connect the (ichache)request port to the (l1chache) reponse port
#system.cpu.icache_port = system.l1_cache.cpu_side

# Can also connect port to array of ports with '='
#   In this case a new port will be spawned on the array
# Connect (icache) request port to (cpu) response port array
#system.cpu.icache_port = system.membus.cpu_side_ports

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
system.workload = SEWorkload.init_compatible(binary)

# We will be executing in syscall emulation (SE) mode 
#   - Focuses on simulating the CPU nd memory system
#   Full system (FS) mode is the alternative
#       - Emulates the entire system and runs an unmodified kernel
#       - Use if you need high fidelity or OS interaction like page table walks
# There is already a compiled helloworld program and we will use that
# We can specify any statically compiled X86 binary though

# Create and set processes command to command we want to run
process = Process()

# This is similar to argv
#   - executable in first position and arguements as the rest of the list
process.cmd = [binary]

# Set spu to use process as its workload
system.cpu.workload = process

# Create functional execution context
system.cpu.createThreads()

# Instantiate the system
root = Root(full_system = False, system = system)
m5.instantiate()

# Begin execution
print("Beginning simulation!")
exit_event = m5.simulate()

# When system finishes inspect the state of the system
print('Exiting @ tick {} because {}'
      .format(m5.curTick(), exit_event.getCause()))

