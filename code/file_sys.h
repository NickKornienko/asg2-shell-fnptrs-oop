// $Id: file_sys.h,v 1.13 2022-01-26 16:10:48-08 - - $
// James Garrett jaagarre
// Nick Kornienko nkornien

#ifndef INODE_H
#define INODE_H

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
using namespace std;

#include "util.h"

// inode_t -
//    An inode is either a directory or a plain file.

enum class file_type
{
   PLAIN_TYPE,
   DIRECTORY_TYPE
};
class inode;
class base_file;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;
using base_file_ptr = shared_ptr<base_file>;
using directory_entries = map<string, inode_ptr>;
using dirent_type = directory_entries::value_type;
ostream &operator<<(ostream &, file_type);

// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.

class inode_state
{
   friend class inode;
   friend ostream &operator<<(ostream &out, const inode_state &);

private:
   inode_ptr root{nullptr};
   inode_ptr cwd{nullptr};
   string prompt_{"% "};

public:
   void set_prompt_(string p);
   void change_pwd();
   inode_ptr get_cwd();
   inode_ptr get_root();
   void set_cwd(inode_ptr);
   inode_state(const inode_state &) = delete;            // copy ctor
   inode_state &operator=(const inode_state &) = delete; // op=
   inode_state();
   const string &prompt() const;
   void prompt(const string &);
   const inode_ptr get_root() const { return root; }
};

// class inode -
// inode ctor -
//    Create a new inode of the given type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
//

class inode
{
   friend class inode_state;

private:
   static size_t next_inode_nr;
   size_t inode_nr;
   base_file_ptr contents;

public:
   bool is_directory();
   base_file_ptr get_contents();
   inode() = delete;
   inode(const inode &) = delete;
   inode &operator=(const inode &) = delete;
   inode(file_type);
   size_t get_inode_nr() const;
   directory_entries &get_dirents();
};

// class base_file -
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.

class file_error : public runtime_error
{
public:
   explicit file_error(const string &what);
};

class base_file
{
   friend class inode_state;
   friend class inode;

protected:
   base_file() = default;
   virtual const string &file_type() const = 0;

public:
   virtual ~base_file() = default;
   base_file(const base_file &) = delete;
   base_file &operator=(const base_file &) = delete;
   virtual size_t size() const = 0;
   virtual const wordvec &readfile() const;
   virtual void writefile(const wordvec &newdata);
   virtual void remove(const string &filename);
   virtual inode_ptr mkdir(const string &dirname);
   virtual inode_ptr mkfile(const string &filename);
   virtual directory_entries &get_dirents();
};

// class plain_file -
// Used to hold data.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
// writefile -
//    Replaces the contents of a file with new contents.

class plain_file : public base_file
{
private:
   wordvec data;
   virtual const string &file_type() const override
   {
      static const string result = "plain file";
      return result;
   }
   size_t file_size = 0;
public:
   virtual size_t size() const override;
   virtual const wordvec &readfile() const override;
   virtual void writefile(const wordvec &newdata) override;
};

// class directory -
// Used to map filenames onto inode pointers.
// default ctor -
//    Creates a new map with keys "." and "..".
// remove -
//    Removes the file or subdirectory from the current inode.
//    Throws an file_error if this is not a directory, the file
//    does not exist, or the subdirectory is not empty.
//    Here empty means the only entries are dot (.) and dotdot (..).
// mkdir -
//    Creates a new directory under the current directory and
//    immediately adds the directories dot (.) and dotdot (..) to it.
//    Note that the parent (..) of / is / itself.  It is an error
//    if the entry already exists.
// mkfile -
//    Create a new empty text file with the given name.  Error if
//    a dirent with that name exists.

class directory : public base_file
{
private:
   // Must be a map, not unordered_map, so printing is lexicographic
   directory_entries dirents;
   virtual const string &file_type() const override
   {
      static const string result = "directory";
      return result;
   }

public:
   virtual size_t size() const override;
   virtual void remove(const string &filename) override;
   virtual inode_ptr mkdir(const string &dirname) override;
   virtual inode_ptr mkfile(const string &filename) override;
   virtual directory_entries &get_dirents() override;
};

#endif
