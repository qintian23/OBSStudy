/*
 * Copyright (c) 2013 Hugh Bailey <obs.jim@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include "c99defs.h"

/*
 * Generic ini-style config file functions
 *
 * NOTE: It is highly recommended to use the default value functions (bottom of
 * the file) before reading any variables from config files.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct config_data;
typedef struct config_data config_t;

#define CONFIG_SUCCESS 0
#define CONFIG_FILENOTFOUND -1
#define CONFIG_ERROR -2

enum config_open_type {
	CONFIG_OPEN_EXISTING,
	CONFIG_OPEN_ALWAYS,
};

EXPORT config_t *config_create(const char *file);
EXPORT int config_open(config_t **config, const char *file,
		       enum config_open_type open_type);
EXPORT int config_open_string(config_t **config, const char *str);
EXPORT int config_save(config_t *config);
/// <summary>
/// 文件安全保存。先将内容保存在临时文件中，再把临时文件替换配置文件
/// </summary>
/// <param name="config">配置文件实例</param>
/// <param name="temp_ext">临时文件名</param>
/// <param name="backup_ext">备份文件名</param>
/// <returns></returns>
EXPORT int config_save_safe(config_t *config, const char *temp_ext,
			    const char *backup_ext);
EXPORT void config_close(config_t *config);

EXPORT size_t config_num_sections(config_t *config);
EXPORT const char *config_get_section(config_t *config, size_t idx);

EXPORT void config_set_string(config_t *config, const char *section,
			      const char *name, const char *value);
EXPORT void config_set_int(config_t *config, const char *section,
			   const char *name, int64_t value);
EXPORT void config_set_uint(config_t *config, const char *section,
			    const char *name, uint64_t value);
EXPORT void config_set_bool(config_t *config, const char *section,
			    const char *name, bool value);
EXPORT void config_set_double(config_t *config, const char *section,
			      const char *name, double value);
/// <summary>
/// 从配置文件选项找出name相关的值，如果没有则从default选项中找
/// </summary>
/// <param name="config"></param>
/// <param name="section"></param>
/// <param name="name"></param>
/// <returns></returns>
EXPORT const char *config_get_string(config_t *config, const char *section,
				     const char *name);
EXPORT int64_t config_get_int(config_t *config, const char *section,
			      const char *name);
EXPORT uint64_t config_get_uint(config_t *config, const char *section,
				const char *name);
EXPORT bool config_get_bool(config_t *config, const char *section,
			    const char *name);
EXPORT double config_get_double(config_t *config, const char *section,
				const char *name);

EXPORT bool config_remove_value(config_t *config, const char *section,
				const char *name);

/*
 * DEFAULT VALUES
 *
 * The following functions are used to set what values will return if they do
 * not exist.  Call these functions *once* for each known value before reading
 * any of them anywhere else.
 *
 * These do *not* actually set any values, they only set what values will be
 * returned for config_get_* if the specified variable does not exist.
 *
 * You can initialize the defaults programmatically using config_set_default_*
 * functions (recommended for most cases), or you can initialize it via a file
 * with config_open_defaults.
 */
EXPORT int config_open_defaults(config_t *config, const char *file);
/// <summary>
/// 设置字符串选项默认值
/// </summary>
/// <param name="config"></param>
/// <param name="section"></param>
/// <param name="name"></param>
/// <param name="value"></param>
EXPORT void config_set_default_string(config_t *config, const char *section,
				      const char *name, const char *value);
EXPORT void config_set_default_int(config_t *config, const char *section,
				   const char *name, int64_t value);
EXPORT void config_set_default_uint(config_t *config, const char *section,
				    const char *name, uint64_t value);
EXPORT void config_set_default_bool(config_t *config, const char *section,
				    const char *name, bool value);
EXPORT void config_set_default_double(config_t *config, const char *section,
				      const char *name, double value);

/* These functions allow you to get the current default values rather than get
 * the actual values.  Probably almost never really needed */
EXPORT const char *config_get_default_string(config_t *config,
					     const char *section,
					     const char *name);
EXPORT int64_t config_get_default_int(config_t *config, const char *section,
				      const char *name);
EXPORT uint64_t config_get_default_uint(config_t *config, const char *section,
					const char *name);
EXPORT bool config_get_default_bool(config_t *config, const char *section,
				    const char *name);
EXPORT double config_get_default_double(config_t *config, const char *section,
					const char *name);

EXPORT bool config_has_user_value(config_t *config, const char *section,
				  const char *name);
EXPORT bool config_has_default_value(config_t *config, const char *section,
				     const char *name);

#ifdef __cplusplus
}
#endif
