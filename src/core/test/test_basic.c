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
#include <stdio.h>

#include "yimmo.h"
#include "core/ymo_tap.h"
#include "ymo_log.h"

int test_just_log_stuff(void)
{
    ymo_log_level_t level = ymo_log_get_level();
    printf("\tMax log level: %s (%i)\n",
            ymo_log_get_level_name(YMO_LOG_LEVEL_MAX), YMO_LOG_LEVEL_MAX);
    printf("\tCurrent log level: %s (%i)\n",
            ymo_log_get_level_name(level), level);
    ymo_log_set_level(YMO_LOG_TRACE);
    ymo_log_trace("%s", "This is a trace message!");
    ymo_log_trace("Trace messages are on! (level %i)\n", YMO_LOG_TRACE);
    return 0;
}


YMO_TAP_RUN(YMO_TAP_NO_INIT(),
        YMO_TAP_TEST_FN(test_just_log_stuff),
        YMO_TAP_TEST_END()
        )


