#pragma once
#include <cstdio>
#include <vector>
#include <string>

#define MIN(a, b) (((b) < (a)) ? b : a)

// This structure is the header of the entire .BIG archive file
// This structure should only occure in the file once.
typedef struct header_s
{
    char extension[4];    // BIGF or BIG4 - little endian?
    uint32_t size;        // Total size of the file (including these 4 bytes) - little endian
    uint32_t files;       // Number of files in the archive - big endian
    uint32_t index_table; // Total size of the index table in bytes - big endian
} header_t;

// Index entry for each file in the index table
typedef struct FileEntry_s
{
    uint32_t pos;   // File position - big endian
    uint32_t size;  // File size     - big endian
    char *filename; // File name
} FileEntry_t;

class BigArchive
{
	FILE *handle;
	std::string path;
	std::string name;
    size_t filesize;

    // Header information.
    header_t header;

public:
	BigArchive(const std::string &name);
	~BigArchive();

	void Create(const std::vector<std::string> &names);
	void Extract(const char *dest, const char *SpecificFile = NULL);
	std::vector<FileEntry_t*> ListFiles();

	FileEntry_t *FindFileEntry(const std::string &path);
    inline std::string GetFileName() const { return this->name; }
    inline std::string GetFilePath() const { return this->path; }
    inline size_t GetFileSize() const { return this->filesize; }
    inline std::string GetArchiveType() const { return std::string(this->header.extension, 4); }

	std::vector<FileEntry_t*> FileEntries;
};
