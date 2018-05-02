
#pragma once
#pragma warning(disable : 4200)
class blockbuf
{
public:

	blockbuf ();
	~blockbuf ();

	void reset ();
	void setsize (int size);
	void addwrite (int len);
	void addread (int len);
	int getreadsize ();
	int getwritesize ();
	char *getreadbuf ();
	char *getwritebuf ();
	int pushdata (void *data, int len);
	int getdata (void *data, int len);
	bool isempty ();

public:
	int read;
	int write;
	int maxsize;
	class blockbuf *next;
	char buf[0];
};
