# SGX Exception Handling

This document describes Open Enclave SDK specific aspects of SGX Exception handling.
In this document,  `exception` refers to hardware exceptions, known as signals in Linux,
and Vectored Exceptions in Windows. It does not refer to software exceptions in the C++ language.

Open Enclave SDK's exception handling uses the **Two Phase Exception Handling** strategy used by
Intel SGX SDK described in [Exception Handling in the Intel(R) SGX SDK]
(https://github.com/MWShan/linux-sgx/blob/master/docs/DesignDocs/IntelSGXExceptionHandling-Linux.md).

The whitepaper [Exception Handling in Intel® Software Guard Extensions (Intel® SGX) Applications]
(https://software.intel.com/content/www/us/en/develop/download/exception-handling-in-intel-software-guard-extensions-applications.html?wapkw=SGX%20exception)
provides an overview of how HW exceptions are handled in Intel(R) Software Guard Extensions (SGX) Enclaves.

# List of SGX hardware exceptions

The following table lists the kinds of exceptions that are reported to the enclave upon encountering,
and therefore can be handled by registering a vectored exception handler within the enclave.

Name  |Vector # |Description
------|---------|----------------
#DE   | 0       | Divider exception.
#DB   | 1       | Debug exception.
#BP   | 3       | Breakpoint exception.
#BR   | 5       | Bound range exceeded exception.
#UD   | 6       | Invalid opcode execution.
#GP   | 13      | General protection exception. Only reported if SECS.MISCSELECT.EXINFO = 1.
#PF   | 14      | Page protection exception. Only reported if SECS.MISCSELECT.EXINFO = 1.
#MF   | 16      | X87 FPU floating-point error.
#AC   | 17      | Alignment check exceptions.
#XM   | 19      | SIMD floating-point exceptions

# Asynchronous Exit Pointer (AEP)

When a hardware exception occurs within the enclave, the processor exits the enclave and jumps to
the code location specified by Asynchronous Exit Pointer (AEP).

## Configuring the AEP pointer

The AEP pointer is passed to the enclave in the `RCX` when an ecall is made
( the `EENTER` leaf of the `ENCLU` instruction).

[host/sgx/enter.c](https://github.com/openenclave/openenclave/blob/68ae4c62ba75ce344434e034bdc44c69ce38447a/host/sgx/enter.c#L173-L188)
```c
        OE_DEFINE_REGISTER(rax, ENCLU_EENTER);
        OE_DEFINE_REGISTER(rbx, tcs);
        OE_DEFINE_REGISTER(rcx, aep);      // AEP pointer.
        OE_DEFINE_REGISTER(rdx, &ecall_context);
        OE_DEFINE_REGISTER(rdi, arg1);
        OE_DEFINE_REGISTER(rsi, arg2);
        OE_DEFINE_FRAME_POINTER(rbp, OE_FRAME_POINTER_VALUE);


        asm volatile("fxsave %[fx_state] \n\t" // Save floating point state.
                     "pushfq \n\t"             // Save flags.
                     "enclu \n\t"
                     "popfq \n\t"               // Restore flags.
                     "fxrstor %[fx_state] \n\t" // Restore floating point state.
                     : OE_ENCLU_REGISTERS
                     : [fx_state] "m"(fx_state)OE_FRAME_POINTER
                     : OE_ENCLU_CLOBBERED_REGISTERS);
```

## AEP function implementation

When an Asynchronous Exit happens within an enclave, the state of the processor is saved in the State Save Area (SSA) within the enclave.
To prevent leaking of enclave confidential information, all the registers are cleared except the following registers:

- `RIP` points to the AEP function. This makes it look to the operating system as if the hardware exception occurred in the AEP function.
- `RAX` has the value '3' which corresponds to `ERESUME` leaf of the `ENCLU` instruction.
- `RBX` points to the Thread Control State (`TCS`) of the enclave thread that encountered the exception.

The AEP function does not conform to any AEP and must start with the `ENCLU`
instruction (see the **AEP and ptrace** section below for why this must be the case).
That is, if the execution were to be resumed, the control would execute the `ENCLU`
instruction with the `ERESUME` leaf, effectively resuming execution within the enclave
at the point where the exception happened.

```asm
OE_AEP:
.cfi_startproc

// N.B, the AEP must be a ERESUME(ENCLU[3]) directly, otherwise single step
// debugging can't work. When an AEX happens, the rax in processor synthentic
// state will be set to 3, and the rbx will be set to TCS of interrupted
// enclave thread automatically. Host side doesn't need to and shouldn't do
// additional settings.

.aep:

    ENCLU
    ud2

.cfi_endproc
```

## AEP, ptrace and debugging

As described above, whenever a hardware exception happens, control exits the enclace (Asynchronous Exit) and is transferred to the AEP function. To the Operating System, it appears as if the exception happened at the AEP function.

This breaks all the development tools that rely on accurate stack trace information being available - debuggers, profilers etc. For enclaves that support debugging, [`oe_ptrace`](debugger/ptraceLib) library is used to recover the original stack trace and registers in case of an AEX. `oe_ptrace` is loaded via LD_PRELOAD and it extends the behavior of [ptrace](https://linux.die.net/man/2/ptrace) to detect AEX and to recover the original stack trace.

The following check is used by `oe_ptrace` to determine whether an AEX has been encountered:

[debugger/ptraceLib/enclave_context.c](debugger/ptraceLib/enclave_context.c)
```c
bool oe_is_aep(pid_t pid, struct user_regs_struct* regs)
{
    uint32_t op_code;

    // Check if rax matches with ENCLU_ERESUME leaf.
    if (regs->rax != ENCLU_ERESUME)
    {
        return false;
    }

    if (oe_read_process_memory(
            pid, (void*)regs->rip, (char*)&op_code, sizeof(op_code), NULL) != 0)
    {
        return false;
    }

    // Check the op_code matches with ENCLU.
    if ((op_code & 0xffffff) == ENCLU_INSTRUCTION)
    {
        return true;
    }

    return false;
}
```

**Question/TODO**
Is checking RAX == ENCLU_RESUME(3) and *RIP ==ENCLU_INSTRUCTION a sufficient condition?


Related issue: [merge SGX debug patches into upstream glibc](https://github.com/openenclave/openenclave/issues/3064)


### AEP and Incremental Linking

One condition for determining whether an AEX has occurrent is the following condition:

```c
    *(uint16_t) rip == ENCLU_INSTRUCTION
```

This implies that the AEP function must start with the `ENCLU` instruction and must not have any function prolog/epilog. Hence, as seen above, `OE_AEP` just has the `enclu` instruction, followed by an `ud2` to indicate that the control will never reach past the `enclu` instruction.

Care must be taken in case of incremental linking and certain other linking strategies which may introduce a thunk in place of the actual AEP function:

```c
    OE_AEP:
        jmp actual_AEP_implementation
```

In such cases, if OE_AEP is passed as the AEP pointer to the enclave, when AEX happens, control will land at the `jmp` instruction instead of the `ENCLU` instruction. This will
break the assumptions of debuggers and other tools that expect RIP to point to ENCLU instruction in case of an AEX.

This issue is solved by adding a label to the location of the `ENCLU` instruction and then
using the label's address instead of the `AEP` function itself.

[host/sgx/windows/aep.asm](host/sgx/windows/aep.asm)
```asm
    NESTED_ENTRY OE_AEP, _TEXT$00
        END_PROLOGUE


    aep LABEL FAR
        ENCLU
        ud2

        BEGIN_EPILOGUE
        ret
    NESTED_END OE_AEP, _TEXT$00

    PUBLIC OE_AEP_ADDRESS; OE_AEP_ADDRESS
    _DATA SEGMENT
    OE_AEP_ADDRESS DQ aep
    _DATA ENDS
```


# Two Phase exception handling

OE SDK uses a two phase exception handling just like Intel SDK.

 In phase 1 and ecall is made back into the enclave so that it can emulate certain instructions

[host/sgx/exception.c](https://github.com/openenclave/openenclave/blob/d3f77c2172bcce59b630b1952fd52e4a7ed7a840/host/sgx/exception.c#L48-L50)
 ```c
        // Call into enclave first pass exception handler.
        uint64_t arg_out = 0;
        oe_result_t result =
            oe_ecall(enclave, OE_ECALL_VIRTUAL_EXCEPTION_HANDLER, 0, &arg_out);
 ```

## CPUID

The sole instruction that the OE SDK emulates currently is `cpuid`. This is because may crypto libraries use the `cpuid` instruction to determine whether AESNI instructions are available for faster encryption.

See [enclave/core/sgx/cpuid.c](https://github.com/openenclave/openenclave/blob/d3f77c2172bcce59b630b1952fd52e4a7ed7a840/enclave/core/sgx/cpuid.c#L76)

To enable emulating `cpuid` the values of `cpuid` are fetched from the host by making the ocall
[`oe_sgx_get_cpuid_table_ocall`](https://github.com/openenclave/openenclave/blob/master/include/openenclave/edl/sgx/cpu.edl) and caching the results.


Caching the results implies that no all information is accurate and cannot be relied upon by the enclave.


## wrfsbase

The `feature/sgx-lkl-support` branch of OE SDK emulates `wrfsbase` instruction.
This is needed since `sgx-lkl` has a threading library that runs within the enclave and therefore it needs to be able to change the `FS`.


## Exception safety

Currently the exception safety specification for a signal handler within the enclave is missing.
[Signal Safety specification for Vectored Exception Handlers ](https://github.com/openenclave/openenclave/issues/2648)
