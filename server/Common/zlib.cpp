#include "zlib.h"
#include <string.h>
#include <assert.h>
extern "C"
{
#include <zlib/zlib.h>
}

int32 Compress(const char* inBuf,uint32 inSize, const char* outBuf, int32 &outSize)
{
	int32 outLen = 0;
	int32 err = 0;
	z_stream stream;
	// 压缩
	stream.next_in = (z_const Bytef *)inBuf;
	stream.avail_in = inSize;

	stream.next_out = (Bytef *)outBuf;
	stream.avail_out = outSize;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;

	err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	assert(Z_OK == err);
	err = deflate(&stream, Z_FINISH);
	assert(err == Z_STREAM_END);
	outSize = stream.total_out;
	err = deflateEnd(&stream);
	return err;
}

int32 UnCompress(const char* inBuf, uint32 inSize, const char* outBuf, int32 &outSize)
{
	int32 err = 0;
	z_stream stream;

	stream.next_in = (z_const Bytef *)inBuf;
	stream.avail_in = inSize;

	stream.next_out = (Bytef *)outBuf;
	stream.avail_out = outSize;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;

	// zlib
	err = inflateInit(&stream);
	assert(Z_OK == err);

	err = inflate(&stream, Z_FINISH);
	assert(err == Z_STREAM_END);

	outSize = stream.total_out;

	err = inflateEnd(&stream);

	return err;
}