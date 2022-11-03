## Get source code

Our version of KLEE can be found at [link](https://github.com/UnitTestBot/klee/commits/luban-utbot-main).

## Build

There is a step-by-step [guide](https://klee.github.io/build-llvm11/) how to build KLEE by original authors.

## Run

The following flags should be used when calling klee binary.

### Essential flags

| KLEE flag | Description |
|-----------|-------------|
| --execution-mode=error-guided | Enables symbolic execution guidance by input errors from off-the-shelf static analyzer |
| --mock-external-calls | All external calls will be treated symbolically |
| --posix-runtime | POSIX environment is emulated |
| --libc=klee | **To UtBotTeam: why not uclibc?** |
| --skip-not-lazy-and-symbolic-pointers | Lazy initialization optimization; crucial for performance |
| --analysis-file=/path/to/source/from/analyzer.c | Full path to C code in which source and sink to be checked are located |
| --analysis-source=N | Line number N of a source of a potential bug to be checked. Set to 0 to disable (default=0) |
| --analysis-sink=M | Line number M of a sink of a potential bug to be checked |

### Nonessential flags

| KLEE flag | Description |
|-----------|-------------|
| --output-dir=/path/to/some/folder.klee-out | Directory in which to write results. Note that it’s parent folder must exist (e.g., a folder /path/to/some must exist) and this folder must not exist (e.g., a folder /path/to/some/folder.klee-out must not exist). |

#### Nonessential flags to control quality/time

| KLEE flag | Description |
|-----------|-------------|
| --max-depth=N | Only allow N symbolic branches. Set to 0 to disable. |
| --max-time=Ns | Halt execution after N seconds. Set to 0s to disable. |
| --max-solver-time=Ns | N seconds is a maximum amount of time for a single SMT query. Set to 0s to disable. |
| --max-instructions=N | Stop execution after N instructions. Set to 0 to disable. |
| --max-forks=N | Only fork N times. Set to -1 to disable. |
| --max-stack-frames=N | Terminate a symbolic state after N stack frames in symbolic state. Set to 0 to disable. |

### Our KLEE reports results to **stderr** as lines of the form:

`KLEE: WARNING: *verdict* at: *filename*:*error line*:*error column*`

where

* *verdict* is one of: “False Positive”, “False Negative”, “True Positive”
* *filename*, *error line*, *error column* is an error address
  * in case of “True Positive” it is a sink address, which is proven to be reachable
  * in case of “False Negative” it is an error found with program trace which does not match input source-sink request, which is proven to be reachable
  * in case of “False Positive” it is a sink address, which is proven to be unreachable

### Known limitations

Current version supports only `null dereference` type of errors. So, it does not support: `reverse null`, `null returns`, etc.

## Examples

### Example run with a false positive found

Assume

* we have an example C file with `WB_BadCase_field2` function at `/home/user/null_dereference/forward_null/FN_SecB_ForwardNull_filed.c`,
* the off-the-shelf static analyzer gave us source and sink
  * source is at line `45`
  * sink is at line `46`
  * original filepath is `/mnt/d/wsl-ubuntu/test2/forward_null/./FN_SecB_ForwardNull_filed.c`
  * (as in `result_FN_SecB_ForwardNull_filed.xml`)
* klee binary at `/home/klee/build/bin/klee` and
* we want to save output to `/home/user/results/forward_null.klee-out`
* we want to stop run after `120` seconds

We can safely run klee as follows:

```bash
mkdir -p /home/user/results # ensure out folder exists
rm -rf /home/user/results/forward_null.klee-out # clean possible previous runs
cd /home/user/null_dereference/forward_null/
clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone /home/user/null_dereference/forward_null/FN_SecB_ForwardNull_filed.c # obtain bytecode
/home/klee/build/bin/klee --execution-mode=error-guided --mock-external-calls --posix-runtime --libc=klee --skip-not-lazy-and-symbolic-pointers --output-dir=/home/user/results/forward_null.klee-out --max-time=120s --analysis-file=FN_SecB_ForwardNull_filed.c --analysis-source=45 --analysis-sink=46 /home/user/null_dereference/forward_null/FN_SecB_ForwardNull_filed.bc
```

#### Possible output is:

<pre><code>
KLEE: NOTE: Using POSIX model: /home/klee/build/runtime/lib/libkleeRuntimePOSIX64_Debug.bca
KLEE: NOTE: using klee versions of input/output functions: /home/klee/build/runtime/lib/libkleeRuntimeIO_C64_Debug.bca
KLEE: output directory is "/tmp/forward_null.klee-out"
KLEE: Using Z3 solver backend
KLEE: Using Z3 core builder
KLEE: WARNING ONCE: Alignment of memory from call "malloc" is not modelled. Using alignment of 8.
KLEE: WARNING ONCE: calling external: syscall(4, 37768712, 31296464) at runtime/POSIX/fd.c:547 12
<mark>KLEE: WARNING: False Negative at: FN_SecB_ForwardNull_filed.c:42:18</mark>
KLEE: ERROR: FN_SecB_ForwardNull_filed.c:42: memory error: null pointer exception
KLEE: NOTE: now ignoring this error at this location
<mark>KLEE: WARNING: False Negative at: FN_SecB_ForwardNull_filed.c:43:19</mark>
KLEE: ERROR: FN_SecB_ForwardNull_filed.c:43: memory error: null pointer exception
KLEE: NOTE: now ignoring this error at this location
<mark>KLEE: WARNING: False Positive at: /mnt/d/wsl-ubuntu/test2/forward_null/./FN_SecB_ForwardNull_filed.c:46</mark>

KLEE: done: total instructions = 1541
KLEE: done: completed paths = 0
KLEE: done: partially completed paths = 3
KLEE: done: generated tests = 3
</pre></code>

### Example run with a true positive found

Assume

* we have an example C file with `WB_BadCase_field2 function` at `/home/user/null_dereference/forward_null/SecB_ForwardNull.c`,
* the off-the-shelf static analyzer gave us source and sink
  * source is at line `129`
  * sink is at line `130`
  * original filepath is `/mnt/d/wsl-ubuntu/test2/forward_null/SecB_ForwardNull.c`
  * (as in `result_SecB_ForwardNull.xml`)
* klee binary at `/home/klee/build/bin/klee` and
* we want to save output to `/home/user/results/forward_null.klee-out`
* we want to stop run after `120` seconds

We can safely run klee as follows:

```bash
mkdir -p /home/user/results # ensure out folder exists
rm -rf /home/user/results/forward_null.klee-out # clean possible previous runs
cd /home/user/null_dereference/forward_null/
clang -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone /home/user/null_dereference/forward_null/SecB_ForwardNull.c # obtain bytecode
/home/klee/build/bin/klee --execution-mode=error-guided --mock-external-calls --posix-runtime --libc=klee --skip-not-lazy-and-symbolic-pointers --output-dir=/home/user/results/forward_null.klee-out --max-time=120s --analysis-file=/mnt/d/wsl-ubuntu/test2/forward_null/SecB_ForwardNull.c --analysis-source=129 --analysis-sink=130 /home/user/null_dereference/forward_null/SecB_ForwardNull.bc
```

#### Possible output is:

<pre><code>KLEE: NOTE: Using POSIX model: /home/klee/build/runtime/lib/libkleeRuntimePOSIX64_Debug.bca
KLEE: NOTE: using klee versions of input/output functions: /home/klee/build/runtime/lib/libkleeRuntimeIO_C64_Debug.bca
KLEE: output directory is "/home/user/results/forward_null.klee-out"
KLEE: Using Z3 solver backend
KLEE: Using Z3 core builder
KLEE: WARNING: undefined reference to function: memcpy_s
KLEE: WARNING: undefined reference to function: memset_s
KLEE: WARNING: undefined reference to function: printf
KLEE: WARNING ONCE: Alignment of memory from call "malloc" is not modelled. Using alignment of 8.
KLEE: WARNING ONCE: calling external: syscall(4, 38901272, 34546672) at runtime/POSIX/fd.c:547 12
<mark>KLEE: WARNING: True Positive at: /mnt/d/wsl-ubuntu/test2/forward_null/SecB_ForwardNull.c:130</mark>
KLEE: ERROR: SecB_ForwardNull.c:130: memory error: null pointer exception
KLEE: NOTE: now ignoring this error at this location

KLEE: done: total instructions = 1536
KLEE: done: completed paths = 0
KLEE: done: partially completed paths = 2
KLEE: done: generated tests = 2
</code></pre>