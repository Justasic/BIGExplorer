#include <string.h>
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <endian.h>
#include <dirent.h>
#include <vector>
#include <algorithm> // for std::sort
#include <libgen.h> // for dirname, basename

#include "BIGFiles.h"
#include "tinyformat.h"


// The .BIG file format is setup in this order using the above structs:
// .Big file {
//      header_t
//
//      index table {
//          FileEntry_t
//          FileEntry_t
//          ...
//      }
//
//      Data
//      ...
// }
//
// The file cosists of a global header, an index of the embedded files,
// and the actual file data.
//
// This is just a simple example of how the format is structured inside the file.
// This format is the same against "BIG4" and "BIGF" files
// The code below depicts how to decode the file below.
//
// The data is in whatever endianness it was archived into and starts at the position
// given in the FileEntry_t structure and ends where position + file size is.
// You should use the index table for finding all files instead of calculating offsets.

void _mkdir(const char *dir)
{
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);

    if(tmp[len - 1] == '/')
    tmp[len - 1] = 0;

    for(p = tmp + 1; *p; p++)
    {
        if(*p == '/')
        {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);
}

// Normalize windows paths to POSIX paths.
// Windows will accept POSIX paths and fix them automatically.
char *NormalizePath(char *path)
{
    char *orig = path;
    while (*path++)
    {
        if (*path == '\\')
        *path = '/';
    }

    return orig;
}

// Give this function a file path and it will iterate backwards
// to the first available '/' and then make directories to that
// path.
void MakeDirectoryFromPath(const char *path)
{
    // Iterate backwards to find a folders we need to make.
    if (!path)
    	return;

    char *str = NormalizePath(strdup(path));

    size_t len = strlen(str);
    for (size_t i = len; i > 0; --i)
    {
        if (str[i] == '/')
        {
            str[i+1] = 0;
            break;
        }
    }

    _mkdir(str);
    free(str);
}

static inline bool is_directory(const std::string &dir)
{
    struct stat st_buf;
    memset(&st_buf, 0, sizeof(struct stat));

    if (stat(dir.c_str(), &st_buf) != 0)
    	return false;

    if (S_ISDIR(st_buf.st_mode))
    	return true;
    return false;
}

off_t fsize(const std::string &filename)
{
    struct stat st;

    if (stat(filename.c_str(), &st) == 0)
    	return st.st_size;

    fprintf(stderr, "Cannot determine size of %s: %s\n", filename.c_str(), strerror(errno));

    return -1;
}

static inline std::vector<std::string> getdir(const std::string &dir)
{
    std::vector<std::string> files;
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL)
    {
        tfm::printf("Error opening \"%s\": %s (%d)\n", dir, strerror(errno), errno);
        return files;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        const char *dirn = dirp->d_name;
        char *filename = NULL;

        if (!strcmp(dirn, ".") || !strcmp(dirn, ".."))
        	continue;

		std::string str = _("%s/%s", dir, dirn);
		files.push_back(str);
    }

    closedir(dp);
    return files;
}

static inline std::vector<std::string> GPKRecursiveDirectoryList(const std::string &dir)
{
    std::vector<std::string> files;
    std::vector<std::string> tmp;
    if (is_directory(dir))
    {
        files = getdir(dir);
        int32_t iter = 0;
        char *file = NULL;
		for (auto file : files)
        {
            if (is_directory(file))
            {
                std::vector<std::string> temp = GPKRecursiveDirectoryList(file);
                tmp.insert(tmp.end(), temp.begin(), temp.end());
            }
        }
    }

    // Append the vectors together
	files.insert(files.end(), tmp.begin(), tmp.end());

    return files;
}

// Copy data from teh file to an actual file
// source is the source file, srcoffset is the
// offset inside the source file handle, dest is
// the new destination file to copy to.
void CopyFiles(FILE *source, size_t srcoffset, size_t length, FILE *dest)
{
    if (!source || !dest)
    return;

    #define COPY_SIZE 1024

    // Initialize our file size
    size_t sz = 0;
    // Go to the place where the file is in the archive
    fseek(source, srcoffset, SEEK_SET);
    // Allocate a buffer to copy between streams
    uint8_t *buf = new uint8_t[COPY_SIZE];
    // Copy loop
    while (sz < length)
    {
        // How much to copy?
        size_t cpysize = MIN(COPY_SIZE, (length - sz));

        // Clear the buffer
        memset(buf, 0, COPY_SIZE);

        // Copy a chunk
        fread(buf, cpysize, 1, source);
        fwrite(buf, cpysize, 1, dest);

        // Add the size
        sz += cpysize;
    }

    tfm::printf("Copied %ld (%s) bytes\n", sz, GetHighestSize(sz));

    // free our buffer
    delete[] buf;
}

// Used for comparing two FileEntries. this function needs to be improved.
int compare(const void *va, const void *vb)
{
    FileEntry_t *a = *(FileEntry_t**)va, *b = *(FileEntry_t**)vb;
    return strcmp(a->filename, b->filename);
}

namespace std
{
	// Get the absolute path of the file (no symlinks or anything)
	std::string realpath(const std::string &realname)
	{
		// the man page for realpath lied to me.
		// Upon giving realpath an unallocated string, realpath simply
		// returns null and gives no indication that anything is wrong.
		// so instead we allocate a whopping sizeof(PATH_MAX) bytes to
		// fill in a directory. I rather dislike this because it can't
		// handle extremely large paths but whatever.
		char *resolved_path = new char[PATH_MAX];
		errno = 0;
		::realpath(realname.c_str(), resolved_path);
		if (errno || !resolved_path)
		{
			printf("Errno: %s (%d)\n", strerror(errno), errno);
			// Whatever.
			return realname;
		}
		std::string ret = resolved_path;
		delete[] resolved_path;
		return resolved_path;
	}

	// Get the full directory path without the last file bit
	std::string dirname(const std::string &dirname)
	{
		char *fuckyou = strdup(dirname.c_str());
		char *fuckyou2 = ::dirname(fuckyou);
		std::string fuckyou3 = fuckyou2;
		free (fuckyou);
		return fuckyou3;
	}

	// Get only the file bit but not the directory or path
	std::string basename(const std::string &basename)
	{
		char *fuckyou = strdup(basename.c_str());
		char *fuckyou2 = ::basename(fuckyou);
		std::string fuckyou3 = fuckyou2;
		free (fuckyou);
		return fuckyou3;
	}
}


BigArchive::BigArchive(const std::string &path) : corrupt(false)
{
	// Get path information
	this->path = std::realpath(path);
	this->name = std::basename(path);

	// Open the file as read, write in binary
	this->handle = fopen(path.c_str(), "rwb");
	if (!this->handle)
		throw std::system_error();


	// Print file size
	fseek(this->handle, 0, SEEK_END);
	this->filesize = ftell(this->handle);
	rewind(this->handle);

	fread(&this->header, sizeof(header_t), 1, this->handle);

	if (this->GetArchiveType() != "BIGF" && this->GetArchiveType() != "BIG4")
	{
		fprintf(stderr, "Invalid archive: \"%s\"\n", this->GetArchiveType().c_str());
		for (unsigned i = 0; i < this->GetArchiveType().size(); ++i)
		{
			fprintf(stderr, "type[%d]: %c -> 0x%X\n", i, this->GetArchiveType()[i], this->GetArchiveType()[i]);
		}
		// TODO: fix this with a proper throw.
		throw std::system_error();
	}

	if (this->filesize != this->header.size)
	{
		fprintf(stderr, "WARNING: The filesize of the archive differs from the value contained in the archive. Possible corruption of archive file?\n");
		this->corrupt = true;
	}

	// Fix endianness to make this even useful.
	this->header.files = be32toh(this->header.files);
	this->header.index_table = be32toh(this->header.index_table);

	// Print some useful info on the header.
	tfm::printf("EXT: %s\n", this->GetArchiveType());
	// Get the file size
	tfm::printf("File size: %ld (%s)\n", filesize, GetHighestSize(filesize));
	// Print other info.
	tfm::printf("Size: %d (%s)\nFiles: %d\nindex_table: %d (0x%X)\n",
		  this->header.size, GetHighestSize(this->header.size),
		  this->header.files, this->header.index_table, this->header.index_table);

	// Parse the index table first.
	this->FileEntries = this->ListFiles();
}

BigArchive::~BigArchive()
{
	for (auto it : this->FileEntries)
	{
		free(it->filename);
		delete it;
	}

	fclose(this->handle);
}

// Create a big file based on the file list.
// This function will list all directories and subdirs
// to include their file paths into the archive. all
// paths are relative.
void BigArchive::Create(const std::vector<std::string> &names)
{
	FILE *f = this->handle;

    tfm::printf("Create called with %d initial file%c:\n", names.size(), names.size() == 1 ? '\0' : 's');

    // Step 1: Generate a list of files.
	std::vector<std::string> filelist;
	int i = 0;
    for (auto it : names)
    {
        tfm::printf("file[%d]: %s\n", i++, it.c_str());
        std::vector<std::string> temp;
        if (is_directory(it.c_str()))
        {
            temp = GPKRecursiveDirectoryList(it);
			filelist.insert(filelist.end(), temp.begin(), temp.end());
        }
        else
			filelist.push_back(it);
    }

    // Step 2: Remove directories.
	for (std::vector<std::string>::iterator it = filelist.begin(), it_end = filelist.end(); it != it_end; )
    {
        // Remove the directory.
        if (is_directory((*it).c_str()))
        {
            tfm::printf("\"%s\" is a directory, skipping...\n", (*it).c_str());
            filelist.erase(it);
        }

		++it;
    }

    tfm::printf("Found %d files to archive:\n", filelist.size());

    // alright so we got all the files recursively, lets
    // create an archive.

    // Step 3: Write the BIG header.

    // Magic bytes
    fwrite("BIGF", 1, 4, f);
    // Filesize (temporarily 0)
    fseek(f, sizeof(uint32_t), SEEK_CUR);
    // Number of files in the archive.
    uint32_t tmp = htobe32(filelist.size());
    fwrite(&tmp, 1, sizeof(uint32_t), f);
    // Location of the index table (which is EOF so placemark for the moment.)
    fseek(f, sizeof(uint32_t), SEEK_CUR);

    // this will contain the total size of the file table after the header above.
    uint32_t preludebytes = 0;

    // Step 4: Convert all the filenames to a FileEntry_t table, then write the data of the files into
    // the file itself.

	std::vector<FileEntry_t*> files;

	for (auto file : filelist)
    {
        // The big archive does not contain directories.
        if (is_directory(file.c_str()))
        tfm::printf("WARNING: Found directory \"%s\" in file list which shouldn't contain directories!\n", file.c_str());

        FileEntry_t *entry = new FileEntry_t;
        entry->filename = strdup(file.c_str());

        // Temporary.
        entry->pos = 0x0;

        // Get the size of the file
        entry->size = fsize(file.c_str());

        // Add the number of bytes together to increase the table size.
        // this includes the length of filename + null byte + file size integer + file position integer.
        preludebytes += strlen(entry->filename) + 1 + (sizeof(uint32_t) * 2);

        files.push_back(entry);
    }

    // Step 5: Write location of first index entry to the BIGF header.
    // Seek to the index_table integer
    fseek(f, sizeof(header_t) - sizeof(uint32_t), SEEK_SET);
    // Calculate the location and make Big Endian then write to the archive.
    tmp = htobe32(preludebytes + sizeof(header_t));
    fwrite(&tmp, 1, sizeof(uint32_t), f);
    // Seek to where we need to be to start writing files.
    fseek(f, preludebytes + sizeof(header_t), SEEK_SET);

    // Sort the vector.
	std::sort(files.begin(), files.end(), compare);
    //vec_sort(&filev, compare);

    // Debug message
    tfm::printf("Started writing files at 0x%lX\n", ftell(f));

    // Step 6: Start copying files into the archive.
	for (std::vector<FileEntry_t*>::iterator it = files.begin(), it_end = files.end(); it != it_end; it++)
    {
		FileEntry_t *filee = *it;
        FILE *src = fopen(filee->filename, "rb");
        if (!src)
        {
            fprintf(stderr, "Failed to open file \"%s\" for reading: %s (%d)\n", filee->filename, strerror(errno), errno);
            // Remove the file, deallocate the name, and continue.
            files.erase(it);
            free(filee->filename);
            delete filee;
			it_end = files.end();
            continue;
        }

        // Set our file position.
        filee->pos = ftell(f);

        // Message.
        tfm::printf("Writing file \"%s\" (%s) into archive \"%s\" at 0x%X\n",
        			filee->filename, GetHighestSize(filee->size), this->path, filee->pos);

        tfm::printf("Next calculated file position starts at 0x%X\n", filee->pos + filee->size);

        // Copy the file into the archive.
        CopyFiles(src, 0, filee->size, f);

        // Cleanup.
        fclose(src);
    }

    // Seek to just after the BIG header so we can write the index.
    fseek(f, sizeof(header_t), SEEK_SET);

    // Debug message?
    tfm::printf("Writing file index...\n");
    // Step 7: Write the file table.
    uint32_t writtensize = 0, index = 0;

	for (auto filee : files)
    {
        tfm::printf("Writing index entry %d: \"%s\" (%d - %s) which is located at 0x%X \n", index++, filee->filename, filee->size, GetHighestSize(filee->size), filee->pos);
        // Copy the first 8 bytes of the header, reverse it for big
        filee->pos = htobe32(filee->pos);
        filee->size = htobe32(filee->size);
        // Copy filee->size and file->pos to the archive index table
        fwrite(filee, 1, sizeof(uint32_t) * 2, f);
        // Copy the string.
        fwrite(filee->filename, 1, strlen(filee->filename), f);

        writtensize += strlen(filee->filename) + 1 + (sizeof(uint32_t) * 2);

        // Deallocate.
        free(filee->filename);
        delete filee;
    }

    if (writtensize != preludebytes)
    	tfm::printf("WARNING: Written index is %d (%s) bytes smaller than estimated index! (%d - %d or %s - %s)\n",
    		  writtensize - preludebytes, GetHighestSize(writtensize - preludebytes), writtensize, preludebytes,
    		  GetHighestSize(writtensize), GetHighestSize(preludebytes));
    else
    	tfm::printf("Written index and estimated index size %d (%s)\n", writtensize, GetHighestSize(writtensize));

    // Step 8: Return to the header + 4 bytes and write both the total file size
    fseek(f, 0, SEEK_END);
    uint32_t totalsize = ftell(f);
    rewind(f);
    // Skip the "BIGF" magic bytes
    fseek(f, 4, SEEK_SET);
    // Write our bytes.
    fwrite(&totalsize, 1, sizeof(uint32_t), f);

    tfm::printf("Writing finished successfully!\n");
}

// Extract a big file
void BigArchive::Extract(const char *dest, const char *SpecificFile)
{
	FILE *f = this->handle;
	FileEntry_t *fe = nullptr;
	std::vector<FileEntry_t*> files = this->FileEntries;

	if (SpecificFile)
	{
		fe = this->FindFileEntry(SpecificFile);
		if (!fe)
		{
			tfm::printf("Could not find file \"%s\" in archive \"%s\"\n", SpecificFile, this->GetFilePath());
			return;
		}

		// We only want one item extracted, fill this vector with that single item and extract it.
		files.clear();
		files.push_back(fe);
	}


    // Now extract the entries.
    for (auto file : this->FileEntries)
    {
        if (!file->filename)
        	continue;

        tfm::printf("Extracting \"%s\" to ", file->filename);
        // Go to the file's position
        fseek(f, file->pos, SEEK_SET);
        char *path = NULL;
        // figure out the path
        char pbuf[PATH_MAX];
        realpath(dest, pbuf);
        asprintf(&path, "%s/%s", pbuf, file->filename);

        // Fix the path.
        path = NormalizePath(path);

        tfm::printf("\"%s\" ...\n", path);

        // Make sure the folder(s) exist
        MakeDirectoryFromPath(path);

        // Open the file for binary writing
        FILE *f2 = fopen(path, "wb");

        // Copy the data.
        CopyFiles(f, file->pos, file->size, f2);

        free(path);

        // Close our file.
        fclose(f2);
    }
}

std::vector<FileEntry_t*> BigArchive::ListFiles()
{
	FILE *f = this->handle;
	std::vector<FileEntry_t*> ret;

	// Now analyze the index table.
	tfm::printf("Dumping index table:\n\n");

	for (uint32_t i = 0; i < this->header.files; ++i)
	{
		FileEntry_t *fe = new FileEntry_t;
		uint32_t pos = 0;
		uint32_t size = 0;
		std::string str;
		// Get the file position
		fread(&pos, sizeof(uint32_t), 1, f);
		// get the file size
		fread(&size, sizeof(uint32_t), 1, f);


		// Reverse the sizes
		fe->pos = be32toh(pos);
		fe->size = be32toh(size);

		// Now read the string, we must do this 1 char at a time
		// because it is a terrible file format. Anyone sane
		// would've included the string length instead of using
		// a null-byte. We have unlimited storage for games now so
		// we don't really need this awful format but whatever.
		while(1)
		{
			uint8_t ch = 0;

			// Read a char, then append it to a string
			if (!fread(&ch, 1, 1, f))
			{
				// Handle an error case so we don't loop forever.
				if (ferror(f) || feof(f))
				{
					tfm::printf("Failed to read next byte in string, exiting loop.\n");
					str = ""; // reset the string in case we had some kind of buffer overflow.
					rewind(f);
					break;
				}
			}
			// Append our char.
			str += ch;

			//tfm::printf("ch: %c 0x%X\n", (ch == 0 ? '@' : ch), ch);

			// Is the char null?
			if (ch == 0)
				break;
		}

		fe->filename = strdup(str.c_str());

		ret.push_back(fe);

		// Print file info.
		tfm::printf(" Index Num: %u\n File: %s\n Size: %u (%s)\n Position: 0x%.7X\n\n",
			  i, fe->filename, fe->size, GetHighestSize(fe->size), fe->pos);
	}

	return ret;
}

FileEntry_t *BigArchive::FindFileEntry(const std::string &path)
{
	for (auto it : this->FileEntries)
	{
		if (!strcmp(path.c_str(), it->filename))
			return it;
	}

	return nullptr;
}
