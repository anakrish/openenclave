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
