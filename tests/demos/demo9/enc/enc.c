// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include "demo_t.h"

#include <stdio.h>
#include <string.h>

#include <openenclave/internal/tests.h>

#include <curl/curl.h>

int enc_demo(char* in)
{
    if (strcmp(in, "curl-demo") != 0)
    {
        return -1;
    }

    {
        OE_TEST(oe_load_module_host_resolver() == OE_OK);
        OE_TEST(oe_load_module_host_socket_interface() == OE_OK);

        //    OE_TEST(oe_load_module_host_epoll() == OE_OK);

        CURL* curl;
        CURLcode res;

        curl = curl_easy_init();
        res = 0;

        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
            /* example.com is redirected, so we tell libcurl to follow
             * redirection */
            //	    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            //    curl_easy_setopt(curl, CURLOPT_HTTP_ONLY, 1L);

            /* Perform the request, res will get the return code */
            res = curl_easy_perform(curl);
            /* Check for errors */
            if (res != CURLE_OK)
                fprintf(
                    stderr,
                    "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            /* always cleanup */
            curl_easy_cleanup(curl);
        }
    }

    printf("Success!\n");
    return 0;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
