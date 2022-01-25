// $Id: virtual-trace.cpp,v 1.21 2022-01-24 14:07:38-08 - - $

//
// Example using objects, with a base object and two derived objects.
// Similar to inheritance2, but uses gcc demangler.
//

#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <vector>
using namespace std;

#define LOG cout << "[" << __PRETTY_FUNCTION__ << "] "

#include <cxxabi.h>
template <typename type>
string demangle_typeid (const type& object) {
   const char* name = typeid(object).name();
   int status = 0;
   unique_ptr<char,decltype(&std::free)> result {
      abi::__cxa_demangle (name, nullptr, nullptr, &status),
      std::free,
   };
   return status == 0 ? result.get() : name;
}


/////////////////////////////////////////////////////////////////
// class object
/////////////////////////////////////////////////////////////////

class object {
   private:
      static size_t next_id;
   protected:
      const size_t id;
      object(); // abstract class, so only derived can used ctor.
   public:
      object (const object&) = delete;            // no copying
      object& operator= (const object&) = delete; // no copying
      virtual ~object(); // must be virtual
      virtual ostream& print (ostream&) const;
      virtual double area() const = 0;
};
size_t object::next_id = 0;

ostream& operator<< (ostream& out, const object& obj) {
   return obj.print (out);
}

object::object(): id(next_id++) {
   LOG << *this << endl;
}

object::~object() {
   LOG << *this << endl;
}

ostream& object::print (ostream& out) const {
   return out << static_cast<const void*> (this) << "->"
              << demangle_typeid(*this) << ":id=" << id;
}


/////////////////////////////////////////////////////////////////
// class square 
/////////////////////////////////////////////////////////////////

class square: public object {
   private:
      double width;
   public:
      explicit square (size_t width);
      virtual ~square();
      virtual ostream& print (ostream&) const override;
      virtual double area() const override;
};

square::square (size_t width_): width(width_) {
   LOG << *this << endl;
}

square::~square() {
   LOG << *this << endl;
}

ostream& square::print (ostream& out) const {
   return this->object::print (out) << ", width=" << width;
}

double square::area() const {
   return pow (width, 2.0);
}


/////////////////////////////////////////////////////////////////
// class circle 
/////////////////////////////////////////////////////////////////

class circle: public object {
   private:
      double radius;
   public:
      explicit circle (size_t radius);
      virtual ~circle();
      virtual ostream& print (ostream&) const override;
      virtual double area() const override;
};

circle::circle (size_t radius_): radius(radius_) {
   LOG << *this << endl;
}

circle::~circle() {
   LOG << *this << endl;
}

ostream& circle::print (ostream& out) const {
   return this->object::print (out) << ", radius=" << radius;
}

double circle::area() const {
   return M_PI * pow (radius, 2.0);
}


/////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////

int main() {

   vector<shared_ptr<object>> vec;
   LOG << "Before push_back ..." << endl;
   vec.push_back (make_shared<circle> ( 5));
   vec.push_back (make_shared<square> ( 5));
   vec.push_back (make_shared<circle> (10));
   vec.push_back (make_shared<square> (10));
   cout << endl;

   LOG << "Before for first for loop ..." << endl;
   for (const auto& ptr: vec) {
      LOG << *ptr << " ...area=" << ptr->area() << endl;
   }
   cout << endl;

   LOG << "Before pop_back for loop ..." << endl;
   vec.pop_back();
   vec.pop_back();
   cout << endl;

   LOG << "Before return 0 ..." << endl;
   return 0;
}

//TEST// valgrind virtual-trace >virtual-trace.out 2>&1
//TEST// mkpspdf virtual-trace.ps virtual-trace.cpp virtual-trace.out

