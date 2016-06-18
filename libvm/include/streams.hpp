#pragma once 
#include <stddef.h>
#include <stdio.h>

enum class SeekMode
{
	Start,
	Current,
	End,
};

class BaseStream
{
public:
	virtual void rewind() = 0;
	virtual void seek(SeekMode mode, long int offset) = 0;
	virtual size_t position() const = 0;
};

class InputStream : 
	public BaseStream
{
public:
	virtual size_t read(void *ptr, size_t size) = 0;
};

class OutputStream : 
	public BaseStream
{
public:
	virtual size_t write(void const *ptr, size_t size) = 0;
};

class FileStream : 
	public virtual InputStream,
	public virtual OutputStream
{
private:
	FILE *file;
public:
	FileStream(const char *fileName, const char *mode);
	~FileStream();
	void rewind() override;
	void seek(SeekMode mode, long int offset) override;
	size_t position() const override;
	
	size_t read(void *ptr, size_t size) override;
	
	size_t write(void const *ptr, size_t size) override;
};
