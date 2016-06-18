#include "include/streams.hpp"
#include <stdio.h>

FileStream::FileStream(const char *fileName, const char *mode) : 
	file(fopen(fileName, mode))
{

}

FileStream::~FileStream()
{
	fclose(this->file);
}

void FileStream::rewind()
{
	::rewind(this->file);
}

void FileStream::seek(SeekMode mode, long int offset)
{
	int sm;
	switch(mode) {
		case SeekMode::Start: sm = SEEK_SET; break;
		case SeekMode::Current: sm = SEEK_CUR; break;
		case SeekMode::End: sm = SEEK_END; break;
	}
	fseek(this->file, offset, sm);
}

size_t FileStream::position() const
{
	return ftell(this->file);
}

size_t FileStream::read(void *ptr, size_t size)
{
	fread(ptr, size, 1, this->file);
	return this->position();
}

size_t FileStream::write(void const *ptr, size_t size)
{
	fwrite(ptr, size, 1, this->file);
	return this->position();
}