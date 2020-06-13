// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <limits.h>
#include <openenclave/host.h>
#include <openenclave/internal/error.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "terminate_u.h"

static oe_enclave_t* _enclave;

oe_result_t host_terminate()
{
    // Terminate from within an ocall should fail.
    return oe_terminate_enclave(_enclave);
}

std::atomic<uint64_t> _num_terminate_failures(0);
std::condition_variable _wake_cv;
std::mutex _mutex;
static volatile bool _waiting = false;
static volatile bool _quit = false;

void host_wait()
{
    // Wait on condition variable.
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_quit)
    {
        _waiting = true;
        _wake_cv.wait(lock);
    }
}

int main(int argc, const char* argv[])
{
    oe_result_t result;
    oe_enclave_t* enclave = NULL;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    const uint32_t flags = oe_get_create_flags();

    if ((result = oe_create_terminate_enclave(
             argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave)) != OE_OK)
        oe_put_err("oe_create_enclave(): result=%u", result);

    _enclave = enclave;

    // Try to terminate an enclave from an ocall.
    // That should fail with a reentrant ecall.
    OE_TEST(enc_check_ocall_terminate(enclave, &result) == OE_OK);
    OE_TEST(result == OE_REENTRANT_ECALL);

    // Launch a thread and make the enclave wait in the host.
    std::thread threads[NUM_TCS];
    threads[0] =
        std::thread([]() { OE_TEST(enc_wait_in_host(_enclave) == OE_OK); });

    //
    while (!_waiting)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // One thread will keep waiting in the host. All other thread will
    // try terminating the enclave, of which only one will succeed and all
    // others will fail.
    const uint64_t EXPECTED_FAILURES = NUM_TCS - 2;
    for (size_t i = 1; i < NUM_TCS; ++i)
    {
        threads[i] = std::thread([]() {
            // Try terminating the enclave. Upon failure, increment failure
            // count.
            oe_result_t result = oe_terminate_enclave(_enclave);
            printf("result = %s\n", oe_result_str(result));
            fflush(stdout);
            if (result == OE_ENCLAVE_TERMINATING)
            {
                if (++_num_terminate_failures == EXPECTED_FAILURES)
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _quit = true;
                    _wake_cv.notify_one();
                }
            }
        });
    }
    // Wait for all threads to finish.
    for (size_t i = 0; i < NUM_TCS; ++i)
        threads[i].join();

    printf("=== passed all tests (terminate)\n");

    return 0;
}
