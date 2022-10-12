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

#include <string.h>

#include "../util/bmem.h"
#include "../util/base.h"

#include "calldata.h"

/*
 *   Uses a data stack.  Probably more complex than it should be, but reduces
 * fetching.
 *
 *   Stack format is:
 *     [size_t    param1_name_size]
 *     [char[]    param1_name]
 *     [size_t    param1_data_size]
 *     [uint8_t[] param1_data]
 *     [size_t    param2_name_size]
 *     [char[]    param2_name]
 *     [size_t    param2_data_size]
 *     [uint8_t[] param2_data]
 *     [...]
 *     [size_t    0]
 *
 *   Strings and string sizes always include the null terminator to allow for
 * direct referencing.
 */

/// <summary>
/// 获取param1_name或者param1_data
/// </summary>
/// <param name="pos">源位置</param>
/// <param name="ptr">目标位置</param>
/// <param name="size">拷贝大小</param>
static inline void cd_serialize(uint8_t **pos, void *ptr, size_t size)
{
	memcpy(ptr, *pos, size);
	*pos += size;
}
/// <summary>
/// 获取param1_name_size或者param1_data_size
/// </summary>
/// <param name="pos">源位置</param>
/// <returns>大小</returns>
static inline size_t cd_serialize_size(uint8_t **pos)
{
	size_t size = 0;
	memcpy(&size, *pos, sizeof(size_t));
	*pos += sizeof(size_t);
	return size;
}
/// <summary>
/// 获取字符串存储位置
/// </summary>
/// <param name="pos"></param>
/// <returns></returns>
static inline const char *cd_serialize_string(uint8_t **pos)
{
	size_t size = cd_serialize_size(pos);
	const char *str = (const char *)*pos;

	*pos += size;

	return (size != 0) ? str : NULL;
}
/// <summary>
/// 检索calldata中是否指定变量，如果有，pos则移动到变量位置；如果没有，pos则移动到当前数据尾部的位置
/// </summary>
/// <param name="data">calldata</param>
/// <param name="name">变量名称</param>
/// <param name="pos">存储变量位置</param>
/// <returns></returns>
static bool cd_getparam(const calldata_t *data, const char *name, uint8_t **pos)
{
	size_t name_size;

	if (!data->size)
		return false;

	*pos = data->stack;

	name_size = cd_serialize_size(pos);
	while (name_size != 0) {
		const char *param_name = (const char *)*pos;
		size_t param_size;

		*pos += name_size;
		if (strcmp(param_name, name) == 0)
			return true;

		param_size = cd_serialize_size(pos);
		*pos += param_size;

		name_size = cd_serialize_size(pos);
	}

	*pos -= sizeof(size_t);
	return false;
}
/// <summary>
/// 拷贝字符串和字符串长度到calldata_t::stack中
/// </summary>
/// <param name="pos">目标位置</param>
/// <param name="str">源字符串</param>
/// <param name="len">源字符串长度</param>
static inline void cd_copy_string(uint8_t **pos, const char *str, size_t len)
{
	if (!len)
		len = strlen(str) + 1;

	memcpy(*pos, &len, sizeof(size_t));
	*pos += sizeof(size_t);
	memcpy(*pos, str, len);
	*pos += len;
}
/// <summary>
/// 拷贝数据和数据大小到calldata_t::stack中
/// </summary>
/// <param name="pos">目标位置</param>
/// <param name="in">源数据</param>
/// <param name="size">源数据大小</param>
static inline void cd_copy_data(uint8_t **pos, const void *in, size_t size)
{
	memcpy(*pos, &size, sizeof(size_t));
	*pos += sizeof(size_t);

	if (size) {
		memcpy(*pos, in, size);
		*pos += size;
	}
}
/// <summary>
/// 首次设置数据，对calldata::stack内存分配，容量大小设置
/// </summary>
/// <param name="data"></param>
/// <param name="name"></param>
/// <param name="in"></param>
/// <param name="size"></param>
static inline void cd_set_first_param(calldata_t *data, const char *name,
				      const void *in, size_t size)
{
	uint8_t *pos;
	size_t capacity;
	size_t name_len = strlen(name) + 1;

	capacity = sizeof(size_t) * 3 + name_len + size;
	data->size = capacity;

	if (capacity < 128)
		capacity = 128;

	data->capacity = capacity;
	data->stack = bmalloc(capacity);

	pos = data->stack;
	cd_copy_string(&pos, name, name_len);
	cd_copy_data(&pos, in, size);
	memset(pos, 0, sizeof(size_t));
}
/// <summary>
/// 检索calldata 容量是否满足，若不满足，在栈模式下不做处理，在堆模式下扩大一倍容量
/// </summary>
/// <param name="data">calldata</param>
/// <param name="pos">当前数据尾部的位置</param>
/// <param name="new_size">当前所需容量大小</param>
/// <returns></returns>
static inline bool cd_ensure_capacity(calldata_t *data, uint8_t **pos,
				      size_t new_size)
{
	size_t offset;
	size_t new_capacity;

	if (new_size < data->capacity)
		return true;
	if (data->fixed) {
		blog(LOG_ERROR, "Tried to go above fixed calldata stack size!");
		return false;
	}

	offset = *pos - data->stack;

	new_capacity = data->capacity * 2;
	if (new_capacity < new_size)
		new_capacity = new_size;

	data->stack = brealloc(data->stack, new_capacity);
	data->capacity = new_capacity;

	*pos = data->stack + offset;
	return true;
}

/* ------------------------------------------------------------------------- */

bool calldata_get_data(const calldata_t *data, const char *name, void *out,
		       size_t size)
{
	uint8_t *pos;
	size_t data_size;

	if (!data || !name || !*name)
		return false;

	if (!cd_getparam(data, name, &pos))
		return false;

	data_size = cd_serialize_size(&pos);
	if (data_size != size)
		return false;

	memcpy(out, pos, size);
	return true;
}

void calldata_set_data(calldata_t *data, const char *name, const void *in,
		       size_t size)
{
	uint8_t *pos = NULL;

	if (!data || !name || !*name)
		return;

	if (!data->fixed && !data->stack) {// 第一次插入数据
		cd_set_first_param(data, name, in, size);
		return;
	}

	if (cd_getparam(data, name, &pos)) {// 已存在数据情况
		size_t cur_size;
		memcpy(&cur_size, pos, sizeof(size_t));

		if (cur_size < size) {
			size_t offset = size - cur_size;
			size_t bytes = data->size;

			if (!cd_ensure_capacity(data, &pos, bytes + offset))
				return;
			memmove(pos + offset, pos, bytes - (pos - data->stack));// 尾部数据移动
			data->size += offset;

		} else if (cur_size > size) {
			size_t offset = cur_size - size;
			size_t bytes = data->size - offset;

			memmove(pos, pos + offset, bytes - (pos - data->stack));// 尾部数据移动
			data->size -= offset;
		}

		cd_copy_data(&pos, in, size);

	} else {// 未存在数据情况
		size_t name_len = strlen(name) + 1;
		size_t offset = name_len + size + sizeof(size_t) * 2;
		if (!cd_ensure_capacity(data, &pos, data->size + offset))
			return;
		data->size += offset;

		cd_copy_string(&pos, name, 0);
		cd_copy_data(&pos, in, size);
		memset(pos, 0, sizeof(size_t));
	}
}

bool calldata_get_string(const calldata_t *data, const char *name,
			 const char **str)
{
	uint8_t *pos;
	if (!data || !name || !*name)
		return false;

	if (!cd_getparam(data, name, &pos))
		return false;

	*str = cd_serialize_string(&pos);
	return true;
}
