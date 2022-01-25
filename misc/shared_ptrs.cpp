// $Id: shared_ptrs.cpp,v 1.63 2022-01-14 20:43:06-08 - - $

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
using namespace std;

// Illustrate use of shared pointers and copying.

const string indent (5, ' ');

#define SHOWBOX { cout << indent << __PRETTY_FUNCTION__ << ": " \
                       << *this << endl; }

using sbox_ptr = shared_ptr<struct sbox>;

class sbox {
   friend ostream& operator<< (ostream&, const sbox&);
   private:
      string value {"<EMPTY>"};
   public:
      sbox() {SHOWBOX}
      sbox (const sbox& that): value(that.value) {SHOWBOX} // copier
      sbox (sbox&& that): value(that.value) {SHOWBOX}      // mover
      sbox& operator= (const sbox& that); // copier
      sbox& operator= (sbox&& that);      // mover
      ~sbox() {SHOWBOX}
      sbox (const string& val): value(val) {SHOWBOX}
      const string& operator*() const { return value; }
};

sbox& sbox::operator= (const sbox& that) {
   if (this != &that) value = that.value;
   SHOWBOX;
   return *this;
}

sbox& sbox::operator= (sbox&& that) {
   if (this != &that) value = that.value;
   SHOWBOX;
   return *this;
}

ostream& operator<< (ostream& out, const sbox& box) {
   return out << &box << ":sbox(\"" << box.value << "\")";
}


template <typename Type>
ostream& operator<< (ostream& out, shared_ptr<Type> ptr) {
   return out << "{" << ptr.get() << "," << ptr.use_count() << "}";
}

#define LINE "[" << __LINE__ << "] "
#define SHOW(STMT) cout << LINE << #STMT << endl; STMT;

void show_ptr (const sbox_ptr& ptr) {
   cout << indent << ptr << " -> ";
   if (ptr) cout << *ptr; else cout << "nullptr";
   cout << endl;
}

int main() {
   SHOW( sbox_ptr junk {make_shared<sbox> (":junk:")}; )
   SHOW( junk = nullptr; )
   SHOW( sbox_ptr a {make_shared<sbox>()}; )
   SHOW( show_ptr(a); )
   SHOW( sbox_ptr b {make_shared<sbox> ("foobar")}; )
   SHOW( auto single {make_shared<sbox> ("single")}; )
   SHOW( show_ptr(single); )
   SHOW( show_ptr(b); )
   SHOW( a = b; )
   SHOW( show_ptr(a); )
   SHOW( show_ptr(b); )
   SHOW( sbox_ptr c {a}; )
   SHOW( show_ptr(c); )
   SHOW( b = nullptr; )
   SHOW( show_ptr(b); )
   SHOW( show_ptr(a); )
   SHOW( return 0; )
}

//TEST// valgrind shared_ptrs >shared_ptrs.out 2>&1
//TEST// mkpspdf shared_ptrs.ps shared_ptrs.cpp shared_ptrs.out

