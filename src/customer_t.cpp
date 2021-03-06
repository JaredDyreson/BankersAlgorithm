//#include "../includes/banker_t.hpp"
#include "../includes/customer_t.hpp"
#include "../includes/extended_vector_t.hpp"

#include <vector>
#include <iostream>

#include <stdio.h> // for printf
#include <stdlib.h>  // for EXIT_SUCCESS
#include <ctime> // std::time


void print_vector(std::vector<int> container) {
  std::cout << '[';
  for(size_t i = 0; i < container.size(); ++i){
    if(i+1 < container.size()){
      std::cout << container[i] << ' ';
    } else {
      std::cout << container[i];
    }
  }
  std::cout << ']';
}


customer_t::customer_t(EVec::extended_vector_t<int> init, EVec::extended_vector_t<int> request, int number){
  this->initial_allocation = init;
  this->need = request;
  this->maximum = this->need + this->initial_allocation;
  this->number = number;
}

customer_t::customer_t() {
  // default is set to 3 resources
  this->initial_allocation = EVec::extended_vector_t<int>({0, 0, 0});
  this->need = EVec::extended_vector_t<int>({0, 0, 0}); 
  this->maximum = EVec::extended_vector_t<int>({0, 0, 0});

  this->number = -1;
}

int customer_t::get_number() {
  return this->number;
}

EVec::extended_vector_t<int> customer_t::get_init(){
  return this->initial_allocation;
}

EVec::extended_vector_t<int> customer_t::request() {
  /*
   * Fail safe
  */
  if(this->request_.empty()){
    this->generate_request();
  }
  return this->request_;
}

bool customer_t::needs_met(){
  return (this->initial_allocation == this->maximum);
}

EVec::extended_vector_t<int> customer_t::get_maximum(){
  return this->maximum;
}

void customer_t::generate_request(){
  /*
   * Adapted from this source:
   * https://en.cppreference.com/w/cpp/numeric/random/rand
  */

  if(!this->request_.empty()) { return; } // NOTE : we will clear this
  std::srand(std::time(nullptr));
  EVec::extended_vector_t<int> _request;
  for(size_t i = 0; i < this->initial_allocation.size(); ++i) {
    if(this->need[i] == 0){ _request.push_back(0); continue; }
    _request.push_back(
      1 + std::rand()/((RAND_MAX + 1u)/this->need[i])
    );
  }
  this->request_ = _request;
  this->need-=this->request_;
}


void customer_t::print() {
  // FIXME
  print_vector(this->initial_allocation.get_data());
  std::cout << "\t\t";
  print_vector(this->maximum.get_data());
  std::cout << "\t\t";
  print_vector(this->need.get_data()); // need
  std::cout << std::endl;
}

void customer_t::drop_resources(){
  this->initial_allocation.clear();
  this->maximum.clear();
  this->request_.clear();
  this->need.clear();
}

void customer_t::obtain_resources() {
  this->initial_allocation = this->maximum;
}

std::ostream& operator<<(std::ostream& os, const customer_t& customer) {
  print_vector(customer.initial_allocation.get_data());
  os << "\t\t";
  print_vector(customer.maximum.get_data());
  os << "\t\t";
  print_vector(customer.need.get_data()); // need
  os << std::endl;
  return os;
}
