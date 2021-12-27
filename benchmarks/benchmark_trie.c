/*=============================================================================
 * test/ymo_basic: Test basic functions of libyimmo.
 *
 * Copyright (c) 2014 Andrew Canaday
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include "core/ymo_assert.h"

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_trie.h"
#include "ymo_http_hdr_table.h"
#include "ymo_http.h"
#include "ymo_std_http.h"

/* HACK HACK: so sorry about this. It's temporary. */
#include "ymo_std_http.c"

#include "ymo_benchmark.h"

#define NO_ITERATIONS 10000000
#define YMO_BENCHMARK_TRIE_COMMON_HTTP 1

#if defined(YMO_BENCHMARK_TRIE_COMMON_HTTP) && YMO_BENCHMARK_TRIE_COMMON_HTTP == 1
# define HEADERS ymo_common_http_headers
# define NO_HEADERS ymo_no_common_http_headers
#else
# define HEADERS ymo_std_request_headers
# define NO_HEADERS ymo_no_std_request_headers
#endif /* YMO_BENCHMARK_TRIE_COMMON_HTTP */

int main(int argc, char** argv)
{
    puts("\n\n*** benchmark_trie: ***");
    size_t i;
    size_t hdr_bytes = 0;
    ymo_status_t r_val;
    const char* hdr;
    struct timeval test_time;
    ymo_trie_t* trie = ymo_trie_create();
    ymo_http_hdr_table_t* table = ymo_http_hdr_table_create();
    ymo_oitrie_t* oitrie = NULL;
    ymo_assert(trie != NULL);

    ymo_log_set_level_by_name("TRACE");
    sranddev();
    for( i = 0; i < NO_HEADERS; i++ ) {
        const char* hdr = HEADERS[i];
        /* ymo_log_trace("Inserting header \"%s\"", hdr); */
        r_val = ymo_trie_add_string(trie, hdr);
        ymo_assert(r_val == YMO_OKAY);
        ymo_http_hdr_table_insert(table, hdr, hdr);

        hdr_bytes += strlen(hdr);
    }

    oitrie = ymo_oitrie_create(trie);
    ymo_assert(oitrie != NULL);
    ymo_log_set_level_by_name("WARNING");

    puts("\n\nBenchmarking with:");
    printf("  Number of headers: %zu\n", NO_HEADERS);
    printf("  Number of iterations: %i\n", NO_ITERATIONS);

    /*------------- OITRIE: ----------------*/
    {
        int hdr_id;
        printf("  total headers in bytes: %zu\n", hdr_bytes);
        printf("  ymo_oitrie size in bytes: %zu\n", ymo_oitrie_sizeof(oitrie));
        printf("  hdr_table size in bytes: %zu\n", sizeof(ymo_http_hdr_table_t) +
                ((NO_HEADERS-YMO_HDR_TABLE_POOL_SIZE)
                 * sizeof(ymo_http_hdr_table_node_t)));
        puts("\nResults:");
        printf("%s", "  ymo_oitrie...");
        benchmark_start();
        for( i = 0; i < NO_ITERATIONS; ++i )
        {
            hdr = HEADERS[rand() % NO_HEADERS];
            r_val = ymo_oitrie_get_id( (int*)&hdr_id, oitrie, hdr);
            /* value = HEADERS[hdr_id]; */
        }
        test_time = benchmark_stop();
        printf("%lu.%06lu (last id: %i)\n",
                (long)test_time.tv_sec, (long)test_time.tv_usec, hdr_id);
    }

    /*------------- HDR TABLE: ------------*/
    {
        const char* val;
        printf("%s", "  ymo_http_hdr_table...");
        benchmark_start();
        for( i = 0; i < NO_ITERATIONS; ++i )
        {
            hdr = HEADERS[rand() % NO_HEADERS];
            val = ymo_http_hdr_table_get(table, hdr);
            /* value = HEADERS[hdr_id]; */
        }
        test_time = benchmark_stop();
        printf("%lu.%06lu (last header: \"%s\")\n",
                (long)test_time.tv_sec, (long)test_time.tv_usec, val);
    }

    ymo_trie_free(trie);
    ymo_oitrie_free(oitrie);
    ymo_http_hdr_table_free(table);
    return 0;
}

