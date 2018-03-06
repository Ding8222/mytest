#include <assert.h>
#include <string.h>
#include "blockbuf.h"

blockbuf::blockbuf ()
{
	read = 0;
	write = 0;
	maxsize = 0;
	next = NULL;
}

blockbuf::~blockbuf ()
{
	next = NULL;
}

void blockbuf::reset ()
{
	read = 0;
	write = 0;
}

void blockbuf::setsize (int size)
{
	maxsize = size;
}

void blockbuf::addwrite (int len)
{
	assert(len >= 0);
	assert(maxsize > (write + len));
	write += len;
}

void blockbuf::addread (int len)
{
	assert(len >= 0);
	assert(write >= (read + len));
	read += len;
}

int blockbuf::getreadsize ()
{
	assert(write >= read);
	return write - read;
}

int blockbuf::getwritesize ()
{
	assert(maxsize > write);
	return (maxsize - write - 1);
}

char *blockbuf::getreadbuf ()
{
	assert(write >= read);
	assert(maxsize > read);
	return &buf[read];
}

char *blockbuf::getwritebuf ()
{
	assert(maxsize > write);
	return &buf[write];
}

#ifndef min
#define min(a, b) (((a) < (b))? (a) : (b))
#endif

int blockbuf::pushdata (void *data, int len)
{
	int writesize;
	assert(maxsize > write);
	assert(data != NULL);
	assert(len > 0);
	writesize = min(getwritesize(), len);
	memcpy(&buf[write], data, writesize);
	write += writesize;
	assert(write < maxsize);
	return writesize;
}

int blockbuf::getdata (void *data, int len)
{
	int readsize;
	assert(write >= read);
	assert(data != NULL);
	assert(len > 0);
	readsize = min(getreadsize(), len);
	memcpy(data, &buf[read], readsize);
	read += readsize;
	assert(read <= write);
	return readsize;
}

bool blockbuf::isempty ()
{
	return ((read == 0) && (write == 0));
}


