# FTrace Tips and Differences

## Function Tracer vs Function Graph Tracer

### **Function Tracer (`function`)**

The `function` tracer **only captures function entry events**, not exits.

**What it captures:**
- Function entry: `function_name()`
- Caller information: `function_name() <- caller_function()`
- Timestamp and CPU information

**What it does NOT capture:**
- Function exit events
- Function duration/timing
- Call hierarchy/indentation

**Example output:**
```
<idle>-0       [001] ...2. 60740.111632: nohz_run_idle_balance <-do_idle
<idle>-0       [001] ...2. 60740.111633: tick_nohz_idle_enter <-do_idle
<idle>-0       [001] d..2. 60740.111634: tick_nohz_start_idle <-tick_nohz_idle_enter
```

### **Function Graph Tracer (`function_graph`)**

The `function_graph` tracer captures **both entry and exit events** with timing information.

**What it captures:**
- Function entry: `function_name() {`
- Function exit: `} /* function_name */`
- Function duration (time spent in function)
- Call hierarchy with indentation
- Cumulative time (with `+` prefix)

**Example output:**
```
 1)   1.750 us    |          } /* _raw_spin_unlock_irqrestore */
 1) + 12.875 us   |        } /* remove_wait_queue */
 1)               |        __pm_runtime_resume() {
 1)   3.000 us    |          _raw_spin_lock_irqsave();
```

## Key Differences Summary

| Feature                 | Function Tracer | Function Graph Tracer |
|---------                |---------------- |---------------------- |
| **Entry Events**        | ✅ Yes          | ✅ Yes                |
| **Exit Events**         | ❌ No           | ✅ Yes                |
| **Timing Information**  | ❌ No           | ✅ Yes (duration)     |
| **Call Hierarchy**      | ❌ No           | ✅ Yes (indentation)  |
| **Performance Impact**  | Lower           | Higher                |
| **Trace File Size**     | Smaller         | Larger                |
| **Analysis Complexity** | Simpler         | More detailed         |

## Performance Impact

### Function Tracer
- **Lower overhead** - only traces function entries
- **Faster execution** - minimal impact on system performance
- **Smaller trace files** - less data captured

### Function Graph Tracer
- **Higher overhead** - traces both entry and exit
- **More detailed** - captures timing and hierarchy
- **Larger trace files** - much more data captured

## When to Use Which Tracer

### Use **Function Tracer** when:
- You want **low overhead**
- You only need to **see which functions are called**
- You're doing **high-frequency tracing**
- You want **smaller trace files**
- You're debugging **function call patterns**

### Use **Function Graph Tracer** when:
- You need **timing information**
- You want to see **call hierarchy**
- You're doing **performance analysis**
- You need to **measure function duration**
- You're debugging **performance bottlenecks**

## Practical Examples

### Function Tracer Example
```bash
# Capture function entries only
sudo trace-cmd record -p function -e net:* -o function_trace.dat iperf3 -s

# Output will show:
# iperf3-1234 [001] 12345.123456: tcp_sendmsg <-sock_sendmsg
# iperf3-1234 [001] 12345.123457: tcp_write_xmit <-tcp_sendmsg
# iperf3-1234 [001] 12345.123458: dev_hard_start_xmit <-tcp_write_xmit
```

### Function Graph Tracer Example
```bash
# Capture function entries and exits with timing
sudo trace-cmd record -p function_graph -e net:* -o graph_trace.dat iperf3 -s

# Output will show:
# 1)               |  tcp_sendmsg() {
# 1)   2.500 us    |    tcp_write_xmit();
# 1) + 15.750 us   |  } /* tcp_sendmsg */
```

## Function Graph Tracer Behavior Explanation

### **Function Entry vs Exit Pattern**

1. **Function Entry**: Always shows the function name with `{`
   ```
   3)               |                                      __local_bh_enable_ip() {
   ```

2. **Function Exit**: Shows `}` but **only some have comments** indicating the function name
   ```
   3) # 1372.375 us |                                            } /* ucfg_dp_start_xmit [wlan] */
   ```

### **Why Only Some Function Exits Have Comments**

The function_graph tracer **captures both entry and exit events**, but the comment annotation depends on the **nesting level and context**:

#### **Functions WITH Comments:**
- **Top-level functions** in the call stack
- **Functions that are easily identifiable** by the tracer
- **Functions with clear entry/exit pairs**

#### **Functions WITHOUT Comments:**
- **Deeply nested functions**
- **Functions where the tracer can't easily match** entry to exit
- **Inline functions or optimized functions**
- **Functions that were interrupted** (marked with `!` or `#`)

### **Why This Happens:**

1. **Call Stack Complexity**: When functions are deeply nested, the tracer sometimes loses track of which `}` corresponds to which function entry.

2. **Optimization**: The kernel may optimize certain functions, making it harder for the tracer to maintain the entry/exit mapping.

3. **Interruption**: Functions marked with `!` or `#` (interrupted/preempted) may lose their exit annotations.

4. **Tracer Limitations**: The function_graph tracer has limitations in very complex call stacks.

## Function Graph Tracer Prefix Meanings

### `!` Prefix
The **`!`** prefix indicates that the function execution was **interrupted** or **preempted**. This means:

- The function was running but got interrupted by another process/task
- The timing measurement may not be completely accurate due to the interruption
- The function didn't complete its normal execution flow

**Example:**
```
2) ! 163.250 us  |      } /* dequeue_entities */
```
This shows that `dequeue_entities` took 163.250 μs but was interrupted during execution.

### `#` Prefix  
The **`#`** prefix indicates that the function execution was **interrupted by an interrupt handler**. This means:

- The function was running in kernel space
- An interrupt occurred and the interrupt handler took over
- The timing includes the time spent in the interrupt handler
- This is a more severe form of interruption than preemption

**Example:**
```
1) # 7869.250 us |      } /* schedule */
```
This shows that `schedule` took 7869.250 μs but was interrupted by an interrupt handler during execution.

### Summary
- **No prefix**: Normal function execution timing
- **`+` prefix**: Cumulative time including sub-functions  
- **`!` prefix**: Function was preempted/interrupted
- **`#` prefix**: Function was interrupted by an interrupt handler

## Filtering Functions

### Method 1: Using trace-cmd (Recommended)

#### Filter Specific Functions
```bash
# Capture only specific functions
sudo trace-cmd record -p function_graph -g function1 -g function2 -g function3 -o filtered_trace.dat

# Example: Capture only TCP-related functions
sudo trace-cmd record -p function_graph -g tcp_sendmsg -g tcp_recvmsg -g tcp_write_xmit -o tcp_trace.dat

# Example: Capture network stack functions
sudo trace-cmd record -p function_graph -g dev_hard_start_xmit -g netif_receive_skb -g ip_output -o net_trace.dat
```

#### Filter Functions with Wildcards
```bash
# Capture all functions starting with "tcp_"
sudo trace-cmd record -p function_graph -g "tcp_*" -o tcp_all_trace.dat

# Capture all functions containing "skb"
sudo trace-cmd record -p function_graph -g "*skb*" -o skb_trace.dat

# Capture all functions in a specific module
sudo trace-cmd record -p function_graph -g "*wlan*" -o wlan_trace.dat
```

### Method 2: Using Direct ftrace Interface

#### Set Function Graph Filter
```bash
# Enable function_graph tracer
echo function_graph > /sys/kernel/debug/tracing/current_tracer

# Set filter to specific functions
echo "tcp_sendmsg tcp_recvmsg tcp_write_xmit" > /sys/kernel/debug/tracing/set_graph_function

# Or use wildcards
echo "tcp_*" > /sys/kernel/debug/tracing/set_graph_function

# Start tracing
echo 1 > /sys/kernel/debug/tracing/tracing_on

# Run your test
iperf3 -c localhost -t 10

# Stop tracing
echo 0 > /sys/kernel/debug/tracing/tracing_on

# View trace
cat /sys/kernel/debug/tracing/trace
```

### Method 3: Multiple Function Filters

#### Using Multiple -g Options
```bash
# Capture multiple specific functions
sudo trace-cmd record -p function_graph \
    -g tcp_sendmsg \
    -g tcp_recvmsg \
    -g dev_hard_start_xmit \
    -g netif_receive_skb \
    -g ip_output \
    -g udp_sendmsg \
    -o network_trace.dat
```

#### Using Function Lists
```bash
# Create a function list file
cat > function_list.txt << EOF
tcp_sendmsg
tcp_recvmsg
tcp_write_xmit
dev_hard_start_xmit
netif_receive_skb
ip_output
udp_sendmsg
EOF

# Use the function list
sudo trace-cmd record -p function_graph -F function_list.txt -o network_trace.dat
```

### Method 4: Advanced Filtering

#### Filter by Module
```bash
# Capture functions from specific kernel modules
sudo trace-cmd record -p function_graph -g "*wlan*" -g "*ath*" -o wifi_trace.dat
```

#### Filter by Process
```bash
# Capture functions only when specific process is running
sudo trace-cmd record -p function_graph -g tcp_sendmsg -P iperf3 -o iperf_tcp_trace.dat
```

#### Combine Function and Event Filters
```bash
# Capture specific functions AND specific events
sudo trace-cmd record -p function_graph \
    -g tcp_sendmsg \
    -g tcp_recvmsg \
    -e net:* \
    -e tcp:* \
    -o combined_trace.dat
```

### Method 5: Exclude Functions (Blacklist)

#### Exclude Specific Functions
```bash
# Capture all functions EXCEPT the ones you specify
echo "!tcp_sendmsg !tcp_recvmsg" > /sys/kernel/debug/tracing/set_graph_notrace

# Or use trace-cmd
sudo trace-cmd record -p function_graph -n tcp_sendmsg -n tcp_recvmsg -o filtered_trace.dat
```

## Practical Examples

### Example 1: Network Stack Analysis
```bash
# Capture key network functions
sudo trace-cmd record -p function_graph \
    -g dev_hard_start_xmit \
    -g netif_receive_skb \
    -g ip_output \
    -g ip_forward \
    -g tcp_sendmsg \
    -g tcp_recvmsg \
    -g udp_sendmsg \
    -g udp_recvmsg \
    -o network_stack_trace.dat

# Run iperf
iperf3 -c localhost -t 10

# Stop tracing (Ctrl+C)
```

### Example 2: TCP Only Analysis
```bash
# Capture only TCP functions
sudo trace-cmd record -p function_graph -g "tcp_*" -o tcp_only_trace.dat

# Run iperf
iperf3 -c localhost -t 10

# Stop tracing (Ctrl+C)
```

### Example 3: Specific Function with Wildcards
```bash
# Capture all functions containing "xmit"
sudo trace-cmd record -p function_graph -g "*xmit*" -o xmit_trace.dat

# Run iperf
iperf3 -c localhost -t 10

# Stop tracing (Ctrl+C)
```

## Viewing Filtered Traces

### In KernelShark
```bash
# Open the filtered trace
kernelshark filtered_trace.dat
```

### As Text
```bash
# Convert to text
trace-cmd report filtered_trace.dat > filtered_trace.txt

# View
less filtered_trace.txt
```

## Tips for Effective Filtering

1. **Start Broad**: Begin with wildcards like `tcp_*` to see what functions are available
2. **Narrow Down**: Once you see the functions, filter to specific ones
3. **Use Multiple Filters**: Combine function filters with event filters
4. **Test Filters**: Use `trace-cmd list` to see available functions
5. **Performance**: More specific filters = better performance and smaller trace files

## Check Available Functions

```bash
# List available functions
sudo trace-cmd list -f | grep tcp

# List functions in a specific module
sudo trace-cmd list -f | grep wlan
```

## GUI Tools for FTrace Analysis

### 1. **Trace Compass** (Most Popular)
- **What it is**: Eclipse-based open-source trace analysis tool
- **Features**: 
  - Interactive timeline views
  - Function call graphs
  - CPU usage visualization
  - Memory and I/O analysis
  - Custom views and filters
- **Installation**: Download from [Eclipse Trace Compass](https://www.eclipse.org/tracecompass/)
- **Usage**: Import your ftrace logs directly

### 2. **KernelShark** (Linux-specific)
- **What it is**: GUI tool specifically designed for Linux kernel traces
- **Features**:
  - Timeline visualization
  - Function graph display
  - CPU and task views
  - Built-in filtering
- **Installation**: 
  ```bash
  sudo apt install kernelshark  # Ubuntu/Debian
  sudo yum install kernelshark  # RHEL/CentOS
  ```
- **Usage**: `kernelshark your_trace_file.dat`

### 3. **Perfetto** (Google's Tool)
- **What it is**: Web-based trace analysis platform
- **Features**:
  - Interactive web interface
  - Timeline analysis
  - Function call visualization
  - Real-time trace viewing
- **Usage**: Upload trace files to [Perfetto UI](https://ui.perfetto.dev/)

## Cross-Compilation for libtraceevent and libtracefs

### libtraceevent Cross-Compilation Variables

```bash
# These ARE defined and used:
CONFIG_INCLUDES="-I/path/to/cross/sysroot/usr/include"
CONFIG_FLAGS="-march=armv7-a -mfpu=neon"

# These are NOT defined in this Makefile:
# CONFIG_LIBS (not used anywhere)
# LIBS is defined but only used for linking, not for library paths
```

### libtracefs Cross-Compilation

The libtracefs Makefile uses **pkg-config** to find libtraceevent:

```makefile
TEST_LIBTRACEEVENT = $(shell sh -c "$(PKG_CONFIG) --atleast-version $(LIBTRACEEVENT_MIN_VERSION) libtraceevent > /dev/null 2>&1 && echo y")

ifeq ("$(TEST_LIBTRACEEVENT)", "y")
LIBTRACEEVENT_INCLUDES = $(shell sh -c "$(PKG_CONFIG) --cflags libtraceevent")
LIBTRACEEVENT_LIBS = $(shell sh -c "$(PKG_CONFIG) --libs libtraceevent")
```

### Methods to Specify libtraceevent Path

#### Method 1: Set PKG_CONFIG_PATH (Recommended)
```bash
# Set PKG_CONFIG_PATH to point to your libtraceevent installation
export PKG_CONFIG_PATH="/path/to/libtraceevent/lib/pkgconfig:$PKG_CONFIG_PATH"

# Then build libtracefs
make
```

#### Method 2: Override PKG_CONFIG Variable
```bash
# Point pkg-config to a specific libtraceevent.pc file
make PKG_CONFIG="pkg-config --with-path=/path/to/libtraceevent/lib/pkgconfig"
```

#### Method 3: Set Environment Variables
```bash
# Set the paths directly
export LIBTRACEEVENT_INCLUDES="-I/path/to/libtraceevent/include"
export LIBTRACEEVENT_LIBS="-L/path/to/libtraceevent/lib -ltraceevent"

# Then build
make
```

#### Method 4: Cross-Compilation Example
```bash
# Set cross-compiler and pkg-config
export CROSS_COMPILE=arm-linux-gnueabihf-
export PKG_CONFIG=arm-linux-gnueabihf-pkg-config
export PKG_CONFIG_PATH="/opt/arm-sysroot/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"

# Build libtracefs
make \
  CROSS_COMPILE=arm-linux-gnueabihf- \
  prefix=/opt/arm-sysroot/usr/local \
  PKG_CONFIG_PATH="/opt/arm-sysroot/usr/local/lib/pkgconfig"
```

## Key Points

1. **libtraceevent.pc file**: Make sure you have a `libtraceevent.pc` file in your libtraceevent installation's `lib/pkgconfig/` directory
2. **Minimum version**: libtracefs requires libtraceevent version ≥ 1.8
3. **PKG_CONFIG_PATH**: This is the most reliable method - it tells pkg-config where to find the .pc files
4. **Automatic detection**: The Makefile automatically detects and uses the correct include and library paths via pkg-config

## Raw FTrace Usage

### Basic FTrace Commands

#### Check FTrace Status
```bash
# Check if ftrace is enabled
cat /sys/kernel/debug/tracing/tracing_on

# Check current tracer
cat /sys/kernel/debug/tracing/current_tracer

# Check available tracers
cat /sys/kernel/debug/tracing/available_tracers

# Check available events
cat /sys/kernel/debug/tracing/available_events
```

#### Enable/Disable FTrace
```bash
# Enable ftrace
echo 1 > /sys/kernel/debug/tracing/tracing_on

# Disable ftrace
echo 0 > /sys/kernel/debug/tracing/tracing_on

# Clear trace buffer
echo > /sys/kernel/debug/tracing/trace
```

#### Set Tracer
```bash
# Set function tracer
echo function > /sys/kernel/debug/tracing/current_tracer

# Set function_graph tracer
echo function_graph > /sys/kernel/debug/tracing/current_tracer

# Set nop tracer (disable)
echo nop > /sys/kernel/debug/tracing/current_tracer
```

### Function Tracer Usage

#### Basic Function Tracing
```bash
# Enable function tracer
echo function > /sys/kernel/debug/tracing/current_tracer

# Start tracing
echo 1 > /sys/kernel/debug/tracing/tracing_on

# Run your test
iperf3 -c localhost -t 10

# Stop tracing
echo 0 > /sys/kernel/debug/tracing/tracing_on

# View trace
cat /sys/kernel/debug/tracing/trace
```

#### Filter Functions
```bash
# Set function filter
echo "tcp_sendmsg tcp_recvmsg" > /sys/kernel/debug/tracing/set_ftrace_filter

# Use wildcards
echo "tcp_*" > /sys/kernel/debug/tracing/set_ftrace_filter

# Exclude functions
echo "!tcp_sendmsg" > /sys/kernel/debug/tracing/set_ftrace_notrace

# Clear filter
echo > /sys/kernel/debug/tracing/set_ftrace_filter
```

### Function Graph Tracer Usage

#### Basic Function Graph Tracing
```bash
# Enable function_graph tracer
echo function_graph > /sys/kernel/debug/tracing/current_tracer

# Start tracing
echo 1 > /sys/kernel/debug/tracing/tracing_on

# Run your test
iperf3 -c localhost -t 10

# Stop tracing
echo 0 > /sys/kernel/debug/tracing/tracing_on

# View trace
cat /sys/kernel/debug/tracing/trace
```

#### Filter Function Graph
```bash
# Set function graph filter
echo "tcp_sendmsg tcp_recvmsg" > /sys/kernel/debug/tracing/set_graph_function

# Use wildcards
echo "tcp_*" > /sys/kernel/debug/tracing/set_graph_function

# Exclude functions
echo "!tcp_sendmsg" > /sys/kernel/debug/tracing/set_graph_notrace

# Clear filter
echo > /sys/kernel/debug/tracing/set_graph_function
```

### Event Tracing

#### Enable Events
```bash
# Enable specific events
echo "net:*" > /sys/kernel/debug/tracing/set_event

# Enable multiple event systems
echo "net:* tcp:* udp:*" > /sys/kernel/debug/tracing/set_event

# Enable all events (be careful - very verbose)
echo "*:*" > /sys/kernel/debug/tracing/set_event
```

#### Filter Events
```bash
# Filter events by process
echo "net:*" > /sys/kernel/debug/tracing/set_event
echo "iperf3" > /sys/kernel/debug/tracing/set_event_pid

# Filter events by CPU
echo "net:*" > /sys/kernel/debug/tracing/set_event
echo "1" > /sys/kernel/debug/tracing/set_event_cpu
```

### Buffer Management

#### Set Buffer Size
```bash
# Set buffer size (in KB)
echo 16384 > /sys/kernel/debug/tracing/buffer_size_kb

# Set per-CPU buffer size
echo 8192 > /sys/kernel/debug/tracing/buffer_size_kb
```

#### Clear Buffers
```bash
# Clear trace buffer
echo > /sys/kernel/debug/tracing/trace

# Clear trace_pipe
echo > /sys/kernel/debug/tracing/trace_pipe
```

### Advanced FTrace Features

#### Stack Traces
```bash
# Enable stack traces for functions
echo 1 > /sys/kernel/debug/tracing/options/stacktrace

# Set function filter with stack traces
echo "tcp_sendmsg" > /sys/kernel/debug/tracing/set_ftrace_filter
echo 1 > /sys/kernel/debug/tracing/options/stacktrace
```

#### Function Profiling
```bash
# Enable function profiling
echo 1 > /sys/kernel/debug/tracing/options/func_stack_trace

# View function call counts
cat /sys/kernel/debug/tracing/trace_stat/function0
```

#### Dynamic Tracing
```bash
# Add dynamic probe
echo 'p:myprobe tcp_sendmsg' > /sys/kernel/debug/tracing/kprobe_events

# Enable the probe
echo 1 > /sys/kernel/debug/tracing/events/kprobes/myprobe/enable

# Remove probe
echo '-:myprobe' > /sys/kernel/debug/tracing/kprobe_events
```

## Trace-cmd Usage

### Basic Trace-cmd Commands

#### List Available Options
```bash
# List available tracers
trace-cmd list

# List available events
trace-cmd list -e

# List available functions
trace-cmd list -f

# List functions matching pattern
trace-cmd list -f | grep tcp
```

#### Record Traces
```bash
# Basic recording
sudo trace-cmd record -p function_graph -o trace.dat command

# Record with specific events
sudo trace-cmd record -e net:* -o trace.dat command

# Record with function filters
sudo trace-cmd record -p function_graph -g tcp_sendmsg -o trace.dat command
```

#### Report Traces
```bash
# Convert binary trace to text
trace-cmd report trace.dat

# Save report to file
trace-cmd report trace.dat > trace.txt

# Report with specific filters
trace-cmd report -f "comm == iperf3" trace.dat
```

### Advanced Trace-cmd Features

#### Live Tracing
```bash
# Start live tracing
sudo trace-cmd stream -p function_graph -e net:*

# View live trace
sudo trace-cmd stream -p function_graph -e net:* | head -100
```

#### Trace Analysis
```bash
# Show trace statistics
trace-cmd stat trace.dat

# Show CPU usage
trace-cmd stat -c trace.dat

# Show process information
trace-cmd stat -p trace.dat
```

#### Trace Filtering
```bash
# Filter by process
trace-cmd report -P iperf3 trace.dat

# Filter by CPU
trace-cmd report -C 1 trace.dat

# Filter by time range
trace-cmd report -s 1000 -e 2000 trace.dat
```

### Trace-cmd Examples

#### Network Analysis
```bash
# Capture network events
sudo trace-cmd record -e net:* -e tcp:* -e udp:* -o network.dat iperf3 -s

# Capture network functions
sudo trace-cmd record -p function_graph -g "*net*" -g "*tcp*" -o network_func.dat iperf3 -s

# Capture specific network functions
sudo trace-cmd record -p function_graph \
    -g dev_hard_start_xmit \
    -g netif_receive_skb \
    -g tcp_sendmsg \
    -g tcp_recvmsg \
    -o tcp_analysis.dat iperf3 -s
```

#### Performance Analysis
```bash
# Capture scheduling events
sudo trace-cmd record -e sched:* -o sched.dat stress --cpu 1 --timeout 10s

# Capture memory events
sudo trace-cmd record -e kmem:* -e mm:* -o memory.dat stress --vm 1 --timeout 10s

# Capture I/O events
sudo trace-cmd record -e block:* -e ext4:* -o io.dat dd if=/dev/zero of=/tmp/test bs=1M count=100
```

#### System Analysis
```bash
# Capture system calls
sudo trace-cmd record -e syscalls:* -o syscalls.dat iperf3 -c localhost -t 10

# Capture interrupts
sudo trace-cmd record -e irq:* -o interrupts.dat iperf3 -c localhost -t 10

# Capture all events (very verbose)
sudo trace-cmd record -e "*:*" -o all_events.dat iperf3 -c localhost -t 10
```

### Trace-cmd Configuration

#### Set Default Options
```bash
# Set default buffer size
sudo trace-cmd record -b 10000 -o trace.dat command

# Set default CPU mask
sudo trace-cmd record -c 0,1 -o trace.dat command

# Set default output format
sudo trace-cmd record -o trace.dat command
```

#### Trace-cmd Plugins
```bash
# List available plugins
trace-cmd list -p

# Use specific plugin
sudo trace-cmd record -p function_graph -o trace.dat command
```

### Troubleshooting FTrace

#### Common Issues
```bash
# Check if ftrace is available
ls /sys/kernel/debug/tracing/

# Check if tracing is enabled
cat /sys/kernel/debug/tracing/tracing_on

# Check current tracer
cat /sys/kernel/debug/tracing/current_tracer

# Check available tracers
cat /sys/kernel/debug/tracing/available_tracers
```

#### Permission Issues
```bash
# Run as root
sudo trace-cmd record -o trace.dat command

# Or use sudo for ftrace operations
sudo echo function > /sys/kernel/debug/tracing/current_tracer
```

#### Performance Issues
```bash
# Reduce buffer size for better performance
echo 1024 > /sys/kernel/debug/tracing/buffer_size_kb

# Use function tracer instead of function_graph for lower overhead
echo function > /sys/kernel/debug/tracing/current_tracer

# Filter functions to reduce overhead
echo "tcp_sendmsg tcp_recvmsg" > /sys/kernel/debug/tracing/set_ftrace_filter
```

### FTrace Best Practices

#### Performance Considerations
1. **Use function tracer for high-frequency tracing**
2. **Use function_graph tracer for detailed analysis**
3. **Filter functions to reduce overhead**
4. **Use appropriate buffer sizes**
5. **Monitor system performance during tracing**

#### Analysis Tips
1. **Start with broad filters, then narrow down**
2. **Use KernelShark for complex analysis**
3. **Convert to text format for scripting**
4. **Use trace-cmd report for filtering**
5. **Save traces for later analysis**

#### Common Use Cases
1. **Network debugging**: Use net:* events and tcp_* functions
2. **Performance analysis**: Use function_graph tracer with timing
3. **System calls**: Use syscalls:* events
4. **Memory analysis**: Use kmem:* and mm:* events
5. **I/O analysis**: Use block:* events
