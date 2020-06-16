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

## AEP and ptrace

As described above, whenever a hardware exception happens,

## AEP and Debuggers

## AEP and PLT/Incremental Linking


# Two Phase exception handling

## FS/GS Registers

## Exception safety
