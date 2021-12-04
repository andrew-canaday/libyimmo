/*=============================================================================
 *
 *  Copyright (c) 2014 Andrew Canaday
 *
 *  This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

/*****************************************************************************
 * NOTE: This stuff is all mostly just for fun, okay?
 *       You don't need to do any of this to use the library!
 *****************************************************************************/
#ifndef YIMMO_EXAMPLE_COMMON_NONSENSE_H
#define YIMMO_EXAMPLE_COMMON_NONSENSE_H


const char* GREETING_START =
    "\n\n"
    "*===========================================================================\n"
    "|               __     __\n"
    "|               \\ \\   / /\n"
    "|                \\ \\_/ / ( ) _ __ ___   _ __ ___    ___     \n"
    "|                 \\   /  | || '_ ` _ \\ | '_ ` _ \\  /   \\ [ ]\n"
    "|                  | |   | || | | | | || | | | | || ( ) |   \n"
    "|                  |_|   |_||_| |_| |_||_| |_| |_| \\___/ [ ]\n"
    "|\n"
    "*---------------------------------------------------------------------------\n";

const char* GREETING_END =
    "\n"
    "----------------------------------------------------------------------------"
    "\n\n";


static void issue_startup_msg(void)
{
    puts(GREETING_START);
    printf("Lib version: %i.%i.%i\n",
            YIMMO_VERSION_MAJOR,
            YIMMO_VERSION_MINOR,
            YIMMO_VERSION_PATCH);

    /* Log level info: */
    puts("Logging:");
    printf("  - Log level (max): %s (%i)\n",
            ymo_log_get_level_name(YMO_LOG_LEVEL_MAX), YMO_LOG_LEVEL_MAX);
    char* log_level = getenv("YIMMO_LOG_LEVEL");
    if( log_level ) {
        ymo_log_level_t level = ymo_log_set_level_by_name(log_level);
        printf("  - Log level (YIMMO_LOG_LEVEL=%s): %s (%i)\n",
                log_level, ymo_log_get_level_name(level), level);
    } else {
        ymo_log_level_t level = ymo_log_get_level();
        printf("  - Log level (default): %s (%i)\n",
                ymo_log_get_level_name(level), level);
    }

    /* Server Info: */
    printf("PID: %i\n", (int)getpid());
    puts(GREETING_END);
    return;
}


#endif /* YIMMO_EXAMPLE_COMMON_NONSENSE_H */
