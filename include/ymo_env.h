/*=============================================================================
 * libyimmo: Lightweight socket server framework
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



#ifndef YMO_ENV_H
#define YMO_ENV_H

/** Yimmo Env Utils
 * =================
 */

/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** :param env_name: the name of the environment variable
 * :param value: the value of the env var as a long, if valid
 * :param def_val: an optional default value to use if the env var isn't valid
 * :returns: ``0`` on success; else, the return value is an ``errno`` code.
 *
 */
int ymo_env_as_long(const char* env_name, long* value, long* def_val);


/** :param env_name: the name of the environment variable
 * :param value: the value of the env var as a float, if valid
 * :param def_val: an optional default value to use if the env var isn't valid
 * :returns: ``0`` on success; else, the return value is an ``errno`` code.
 *
 */
int ymo_env_as_float(const char* env_name, float* value, float* def_val);

/** :param env_name: the name of the environment variable
 * :param value: the value of the env var as a double, if valid
 * :param def_val: an optional default value to use if the env var isn't valid
 * :returns: ``0`` on success; else, the return value is an ``errno`` code.
 *
 */
int ymo_env_as_double(const char* env_name, double* value, double* def_val);

#endif /* YMO_ENV_H */

