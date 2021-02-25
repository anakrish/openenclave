// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <inttypes.h>
#include <openenclave/host.h>
#include <openenclave/internal/error.h>
#include <openenclave/internal/tests.h>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <stdio.h>
#include <thread>
#include <unordered_map>
#include "threadArgs.h"

#include "demo_u.h"

int main(int argc, const char* argv[])
{
    oe_result_t result;
    oe_enclave_t* enclave = NULL;
    int return_val;
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    const uint32_t flags = oe_get_create_flags();

    if ((result = oe_create_demo_enclave(
             argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave)) != OE_OK)
        oe_put_err("oe_create_enclave(): result=%u", result);

    result = enc_main(enclave, &return_val, argc-1, argv+1);

    if (result != OE_OK)
        oe_put_err("oe_call_enclave() failed: result=%u", result);

    if (return_val != 0)
        oe_put_err("ECALL failed args.result=%d", return_val);

    result = oe_terminate_enclave(enclave);
    OE_TEST(result == OE_OK);

    printf("=== passed all tests (sqlite)\n");
    return 0;
}

// Host maintains a map of enclave key to host thread ID
static std::unordered_map<uint64_t, std::atomic<std::thread::id>>
    _enclave_host_id_map;
// Host maintains a map of thread id to the thread object
static std::unordered_map<std::thread::id, std::thread> _host_id_thread_map;

void host_exit(int arg)
{
    // Ensure all the threads terminate before the exit
    for (auto& pair : _host_id_thread_map)
    {
        pair.second.join();
    }
    exit(arg);
}

static atomic_flag_lock _host_lock;
struct thread_args_t
{
    oe_enclave_t* enclave;
    uint64_t enc_key;
};

void* host_enclave_thread(void* args)
{
    thread_args_t* thread_args = reinterpret_cast<thread_args_t*>(args);
    // need to cache the values for enc_key and enclave now before _host_lock
    // is unlocked after assigning the thread_id to the _enclave_host_id_map
    // because args is a local variable in the calling method which may exit
    // at any time after _host_lock is unlocked which may cause a segfault
    uint64_t enc_key = thread_args->enc_key;
    oe_enclave_t* enclave = thread_args->enclave;
    std::thread::id thread_id;

    {
        // Using atomic_thread_host_id_map lock to ensure the mapping is updated
        // before the thread_id lookup
        atomic_lock lock(_host_lock);

        std::thread::id thread_id = std::this_thread::get_id();
        OE_TEST(
            _host_id_thread_map.find(thread_id) != _host_id_thread_map.end());

        // Populate the enclave_host_id map with the thread_id
        _enclave_host_id_map[enc_key] = thread_id;
    }

    printf(
        "host_enclave_thread(): enc_key=%" PRIu64
        " has host thread_id of %#10zx\n",
        enc_key,
        std::hash<std::thread::id>()(thread_id));

    // Launch the thread
    oe_result_t result = enc_enclave_thread(enclave, enc_key);
    OE_TEST(result == OE_OK);

    return NULL;
}

void host_create_thread(uint64_t enc_key, oe_enclave_t* enclave)
{
    thread_args_t thread_args = {enclave, enc_key};
    std::thread::id thread_id;
    const std::atomic<std::thread::id>* mapped_thread_id;

    {
        // Using atomic locks to protect the enclave_host_id_map
        // and update the _host_id_thread_map upon the thread creation
        atomic_lock lock(_host_lock);
        mapped_thread_id = &_enclave_host_id_map[enc_key];

        // New Thread is created and executes host_enclave_thread
        std::thread t = std::thread(host_enclave_thread, &thread_args);

        thread_id = t.get_id();
        _host_id_thread_map[thread_id] = std::move(t);
    }

    // Main host thread waits for the enclave id to host id mapping to be
    // updated
    while (*mapped_thread_id == std::thread::id())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Sanity check
    if (thread_id != *mapped_thread_id)
    {
        printf("Host thread id incorrect in the enclave_host_id_map\n");
        abort();
    }
}

int host_join_thread(uint64_t enc_key)
{
    int ret_val = 0;
    std::thread::id thread_id;

    // Find the thread_id from the enclave_host_id_map using the enc_key
    {
        // Using atomic locks to protect the enclave_host_id_map
        atomic_lock lock(_host_lock);
        const auto it = _enclave_host_id_map.find(enc_key);
        if (it != _enclave_host_id_map.end())
        {
            thread_id = it->second;
            lock.unlock();

            auto& t = _host_id_thread_map[thread_id];
            OE_TEST(t.joinable());
            t.join();

            // Update the shared memory only after join
            {
                // Delete the enclave_host_id mapping as the thread_id may be
                // reused in future
                lock.lock();
                _enclave_host_id_map.erase(enc_key);

                printf(
                    "host_join_thread() succeeded for enclave id=%" PRIu64
                    ", host "
                    "id=%#10zx\n",
                    enc_key,
                    std::hash<std::thread::id>()(thread_id));
            }
        }
        else
        {
            printf(
                "host_join_thread() failed to find enclave id=%" PRIu64
                " in host "
                "map\n",
                enc_key);
            abort();
        }
    }

    return ret_val;
}

int host_detach_thread(uint64_t enc_key)
{
    printf("host_detach_thread():enclave key=%" PRIu64 "\n", enc_key);

    // Find the thread_id from the enclave_host_id_map using the enc_key

    // Using atomic locks to protect the enclave_host_id_map
    atomic_lock lock(_host_lock);
    const auto it = _enclave_host_id_map.find(enc_key);
    if (it != _enclave_host_id_map.end())
    {
        std::thread::id thread_id = it->second;
        lock.unlock();

        auto& t = _host_id_thread_map[thread_id];
        t.detach();

        {
            // Delete the _enclave_host_id mapping as the host thread_id may be
            // reused in future
            lock.lock();
            _enclave_host_id_map.erase(enc_key);
        }
        printf(
            "host_detach_thread() enclave id=%" PRIu64 ", host "
            "thread id=%#10zx\n",
            enc_key,
            std::hash<std::thread::id>()(thread_id));
    }
    else
    {
        printf(
            "host_detach_thread() failed to find enclave key=%" PRIu64
            " in host "
            "map\n",
            enc_key);
        abort();
    }
    return 0;
}
