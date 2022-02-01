// $Id: commands.cpp,v 1.27 2022-01-28 18:11:56-08 - - $

#include "commands.h"
#include "debug.h"

const command_hash cmd_hash{
    {"cat", fn_cat},
    {"cd", fn_cd},
    {"echo", fn_echo},
    {"exit", fn_exit},
    {"ls", fn_ls},
    {"lsr", fn_lsr},
    {"make", fn_make},
    {"mkdir", fn_mkdir},
    {"prompt", fn_prompt},
    {"pwd", fn_pwd},
    {"rm", fn_rm},
    {"rmr", fn_rmr},
};

command_fn find_command_fn(const string &cmd)
{
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF('c', "[" << cmd << "]");
   const auto result{cmd_hash.find(cmd)};
   if (result == cmd_hash.end())
   {
      throw command_error(cmd + ": no such command");
   }
   return result->second;
}

command_error::command_error(const string &what) : runtime_error(what)
{
}

int exit_status_message()
{
   int status{exec::status()};
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_cd(inode_state &state, const wordvec &words)
{
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   // if (words[1]) == "")
   // {
   //    state.set_cwd(root);
   // }

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      if (itr->first == words[1])
      {
         state.set_cwd(itr->second);
         return;
      }
   }
   cout << "not found"
        << "\n";

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_echo(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   cout << word_range(words.cbegin() + 1, words.cend()) << endl;
}

void fn_exit(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   throw ysh_exit();
}

void fn_ls(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   DEBUGS(
       'l',
       const auto &dirents = state.get_root()->get_dirents();
       for (const auto &entry
            : dirents) {
          cerr << "\"" << entry.first << "\"->" << entry.second << endl;
       });
}

void fn_lsr(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_make(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_mkdir(inode_state &state, const wordvec &words)
{
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries &dirents = contents->get_dirents();

   dirents.insert(std::pair(words[1], state.get_cwd()->get_contents()->mkdir(words[1])));

   dirents.insert(std::pair(".", state.get_cwd()->get_contents()->mkdir(words[1])));
   dirents.insert(std::pair("..", state.get_cwd()->get_contents()->get_dirents().at(".")));


   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_prompt(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_pwd_recur(inode_ptr cwd_, size_t node_number)
{
   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();
   bool atRoot = false;

   if (dirents.at("..") == cwd_)
   {
      cout << "\\";
      atRoot = true;
   }

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      if (itr->second->get_inode_nr() == node_number)
         cout << itr->first << "\\";
   }

   if (!atRoot)
      fn_pwd_recur(dirents.at(".."), cwd_->get_inode_nr());

   return;
}

void fn_pwd(inode_state &state, const wordvec &words)
{
   (void)words;
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   if (dirents.at("..") == cwd_)
   {
      cout << "\\\n";
      return;
   }

   fn_pwd_recur(dirents.at(".."), cwd_->get_inode_nr());
   cout << "\n";

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_rm(inode_state &state, const wordvec &words)
{
   (void)words;
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      cout << '\t' << itr->first << '\t' << itr->second
           << '\n';
   }
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_rmr(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
}
