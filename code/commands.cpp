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

   if (words.size() < 2)
   {
      exec::status(0);
   }
   else
   {
      try
      {
         exec::status(stoi(words[1]));
      }
      catch (const std::exception &)
      {
         exec::status(127);
      }
   }

   throw ysh_exit();
}

void fn_ls(inode_state &state, const wordvec &words)
{
   inode_ptr old_cwd = state.get_cwd();

   if (words.size() > 1)
   {
      wordvec new_words;
      new_words.push_back("");

      if (words[1] == "/")
      {
         new_words.push_back("");
         fn_cd(state, new_words);
      }
      else
      {
         new_words.push_back(words[1]);
         try
         {
            fn_cd(state, new_words);
            cout << words[1] << "\n";
         }
         catch (file_error &)
         {
            state.set_cwd(old_cwd);
            throw file_error(words[1] + ": No such file or directory");
         }
      }
   }

   wordvec new_words;
   new_words.push_back("ls");
   fn_pwd(state, new_words);

   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      size_t size = itr->second->get_contents()->size();
      size_t i_num = itr->second->get_inode_nr();
      string dir_string = "";
      if(itr->second->is_directory())
         dir_string += "/";

      printf("%6ld  %6ld  ", i_num, size);
      cout << itr->first << dir_string << "\n";
   }

   state.set_cwd(old_cwd);

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_lsr(inode_state &state, const wordvec &words)
{
   inode_ptr old_cwd = state.get_cwd();

   if (words[0] == "lsr" && words.size() > 1)
   {
      wordvec cd_words;
      cd_words.push_back("");
      cd_words.push_back(words[1]);

      try
      {
         fn_cd(state, cd_words);
      }
      catch (file_error &)
      {
         state.set_cwd(old_cwd);
         throw file_error(words[1] + ": No such file or directory");
      }
   }

   wordvec new_words;
   new_words.push_back("");
   fn_ls(state, new_words);

   inode_ptr cwd_ = state.get_cwd();

   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      if (!itr->second->is_directory() ||
          itr->first == "." ||
          itr->first == "..")
         continue;

      new_words.push_back(itr->first);
      fn_cd(state, new_words);
      fn_lsr(state, new_words);
      new_words.pop_back();
      wordvec parent;
      parent.push_back("");
      parent.push_back("..");
      fn_cd(state, parent);
   }

   state.set_cwd(old_cwd);

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
      throw file_error("mkdir: cannot create directory '" +
                       words[1] +
                       "': File exists");
   }

   inode_ptr node =
       state.get_cwd()->get_contents()->mkdir(words[1]);

   dirents.insert(std::pair(words[1], node));

   node = state.get_cwd()->get_contents()->mkdir(words[1]);
   dirents.insert(std::pair(words[1], node));

   inode_ptr child = dirents.at(words[1]);
   directory_entries &child_dirents =
       child->get_contents()->get_dirents();
   child_dirents.insert(std::pair("..", cwd_));

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_mkdir(inode_state &state, const wordvec &words)
{
   if (words.size() < 2)
      throw file_error("mkdir: missing operand");

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
            throw file_error("cannot create directory " +
                             words[i] +
                             ": No such file or directory");
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
   string p = "";

   for (size_t i = 1; i < words.size(); i++)
      p += words[i] + " ";

   state.set_prompt_(p);

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
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   if (dirents.at("..") == cwd_)
   {
      if (words[0] == "ls")
         cout << "/:\n";
      else
         cout << "/\n";

      return;
   }

   fn_pwd_recur(dirents.at(".."), cwd_->get_inode_nr());
   if (words[0] == "ls")
      cout << ":\n";
   else
      cout << "\n";

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_rm_sngl(inode_state &state, const wordvec &words, inode_ptr old)
{
   inode_ptr cwd_ = state.get_cwd();
   base_file_ptr contents = cwd_->get_contents();
   directory_entries &dirents = contents->get_dirents();

   if (!dirents.count(words[1]))
   {
      state.set_cwd(old); 
      throw file_error("rm: cannot remove '" +
                       words[1] +
                       "': No such file or directory");
   }
   if (cwd_->is_directory())
   {
      inode_ptr child = dirents.at(words[1]);
      directory_entries child_dirents =
          child->get_contents()->get_dirents();
      if (child_dirents.size() > 2)
      {
         state.set_cwd(old);
         throw file_error("rm: cannot remove '" +
                          words[1] +
                          "': Is a directory");
      }
   }

   dirents.erase(words[1]);

   DEBUGF('c', state);
   DEBUGF('c', words);
}

void fn_rm(inode_state &state, const wordvec &words)
{
   if (words.size() < 2)
      throw file_error("rm: missing operand");

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
            throw file_error("cannot remove " +
                             words[i] +
                             ": No such file or directory");
            return;
         }
      }

      wordvec final_token;
      final_token.push_back("");
      final_token.push_back(tokens[tokens.size() - 1]);
      fn_rm_sngl(state, final_token, old_cwd);
      state.set_cwd(old_cwd);
   }
}

void fn_rmr(inode_state &state, const wordvec &words)
{
   if (words.size() < 2)
      throw file_error("rmr: missing operand");

   inode_ptr old_cwd = state.get_cwd();

   if (words[0] == "rmr")
   {
      wordvec cd_words;
      cd_words.push_back("");
      cd_words.push_back(words[1]);

      try
      {
         fn_cd(state, cd_words);
      }
      catch (file_error &)
      {
         state.set_cwd(old_cwd);
         throw file_error(words[1] + ": No such file or directory");
      }
   }

   wordvec new_words;
   new_words.push_back("");

   inode_ptr cwd_ = state.get_cwd();

   base_file_ptr contents = cwd_->get_contents();
   directory_entries dirents = contents->get_dirents();

   for (auto itr = dirents.begin(); itr != dirents.end(); ++itr)
   {
      if (!itr->second->is_directory())
      {
         new_words.push_back(itr->first);
         fn_rm(state, new_words);
         new_words.pop_back();
         continue;
      }
      else if (itr->first == "." || itr->first == "..")
         continue;

      new_words.push_back(itr->first);
      fn_cd(state, new_words);
      fn_rmr(state, new_words);
      wordvec parent;
      parent.push_back("");
      parent.push_back("..");
      fn_cd(state, parent);
      fn_rm(state, new_words);
      new_words.pop_back();
   }

   state.set_cwd(old_cwd);
   if(words[0] == "rmr")
      fn_rm(state, words);

   DEBUGF('c', state);
   DEBUGF('c', words);
}
