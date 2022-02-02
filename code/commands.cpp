// $Id: commands.cpp,v 1.27 2022-01-28 18:11:56-08 - - $

#include "commands.h"
#include "debug.h"
#include <sstream>

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

vector<string> tokenize_path(string path)
{
   string segment;
   vector<string> tokens;
   istringstream stream(path);
   while (getline(stream, segment, '/'))
   {
      tokens.push_back(segment);
   }

   return tokens;
}

void fn_cat(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_cd_single(inode_state &state, const wordvec &words)
{
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      if (itr->first == words[1])
      {
         if (!itr->second->is_directory())
         {
            throw file_error("Plain file, not directory");
         }
         state.set_cwd(itr->second);
         return;
      }
   }

   throw file_error("No such file or directory");

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_cd(inode_state &state, const wordvec &words)
{
   if (words.size() > 2)
   {
      throw file_error("More than one operand given");
   }

   if (words.size() == 1)
   {
      state.set_cwd(state.get_root());
      return;
   }

   inode_ptr cwd_ = state.get_cwd();
   inode_ptr old_cwd = state.get_cwd();

   vector<string> tokens = tokenize_path(words[1]);

   for (size_t i = 0; i < tokens.size(); i++)
   {
      wordvec word_vec;
      word_vec.push_back("");
      word_vec.push_back(tokens[i]);
      try
      {
         fn_cd_single(state, word_vec);
      }
      catch (file_error &)
      {
         state.set_cwd(old_cwd);
         throw file_error(words[1] + ": No such file or directory");
         return;
      }
   }
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

void fn_mkdir_single(inode_state &state, const wordvec &words)
{
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries &dirents = contents->get_dirents();

   if (dirents.count(words[1]))
   {
      throw file_error("mkdir: cannot create directory '" + words[1] + "': File exists");
      // cout << "mkdir: cannot create directory '" << words[i] << "': File exists\n";
      // continue;
   }

   dirents.insert(std::pair(words[1], state.get_cwd()->get_contents()->mkdir(words[1])));

   inode_ptr child = dirents.at(words[1]);
   directory_entries &child_dirents = child->get_contents()->get_dirents();
   child_dirents.insert(std::pair("..", cwd_));

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_mkdir(inode_state &state, const wordvec &words)
{
   inode_ptr cwd_ = state.get_cwd();
   inode_ptr old_cwd = state.get_cwd();

   for (size_t i = 1; i < words.size(); i++)
   {
      vector<string> tokens = tokenize_path(words[i]);

      for (size_t j = 0; j < tokens.size() - 1; j++)
      {
         wordvec word_vec;
         word_vec.push_back("");
         word_vec.push_back(tokens[j]);

         try
         {
            fn_cd(state, word_vec);
         }
         catch (file_error &)
         {
            state.set_cwd(old_cwd);
            throw file_error("cannot create directory " + words[i] + ": No such file or directory");
            return;
         }
      }
      wordvec final_token;
      final_token.push_back("");
      final_token.push_back(tokens[tokens.size() - 1]);
      fn_mkdir_single(state, final_token);
      state.set_cwd(old_cwd);
   }
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

   if (dirents.at("..") != cwd_)
      fn_pwd_recur(dirents.at(".."), cwd_->get_inode_nr());

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      if (itr->second->get_inode_nr() == node_number)
      {
         cout << "/" << itr->first;
         break;
      }
   }

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
      cout << "/\n";
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
