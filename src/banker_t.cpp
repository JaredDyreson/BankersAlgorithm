#include "../includes/banker_t.hpp"
#include "../includes/customer_t.hpp"
#include "../includes/extended_vector_t.hpp"
#include "../includes/utils.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept> 
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/*
 * Global variables for the runner function below
*/

pthread_mutex_t mutex_;
banker_t banker_;
bool STATUS = false, ALLOCATION_STATUS;

void change_value(bool* instance, bool value){ *instance = value; }

void* runner(void* parameters) {
  customer_t* customer = (customer_t*)parameters;

  int i = 0;

  MUTEX_SAFE(printf("[INFO] Customer thread p#%d has started..\n",
                 customer->get_number()))


  while (!customer->needs_met() && i < DEADLOCK) {
    int index = customer->get_number();
    bool approved = banker_.can_grant_request(index, customer->request());

    if (approved) {

      MUTEX_SAFE(printf("[APPROVED] Granting process %d the desired resources\n",
                     index))

      banker_.withdrawl_resources(customer);
      if(!ALLOCATION_STATUS) {
        MUTEX_SAFE(printf("[DENIED] Allocation would have lead to an underdraft of resources\n"))
      }

      MUTEX_SAFE(std::cout << banker_ << std::endl)

      if (customer->needs_met()) {
        MUTEX_SAFE(
            printf("[RECIEVED] Process %d has been sataiated and will give its "
                   "resources back\n",
                   index))
        banker_.deposit(customer);
      }

    } else {
      MUTEX_SAFE(printf("[DENIED] Will not grant process %d desired resources\n",
                     index))
    }
    banker_.clear_sequence();
    ++i;
  }
  MUTEX_SAFE(printf("[INFO] Customer thread p#%d has completed..\n",
                 customer->get_number()))
  
  if (i <= DEADLOCK) {
    change_value(&STATUS, true);
    pthread_exit(EXIT_SUCCESS);
  } else {
    change_value(&STATUS, false);
    MUTEX_SAFE(std::cerr << "[FATAL] Algorithm has entered a deadlocked state!"
                         << std::endl)
    pthread_exit(
        (void*)EXIT_FAILURE);  // uhhh what's going on clang, are you drunk?
  }
}


banker_t::banker_t(EVec::extended_vector_t<int> container, std::vector<customer_t*> customers) {
  this->available_funds = container;
  this->customers = customers;
  this->_n_processes = customers.size();
  this->_m_resources = customers[0]->get_init().size();
}

banker_t::banker_t(){
  this->available_funds = EVec::extended_vector_t<int>();
  this->_n_processes = 0;
  this->_m_resources = 0;
}

EVec::extended_vector_t<int> banker_t::get_available_funds() {
  return this->available_funds;
}

bool banker_t::is_safe(int index, EVec::extended_vector_t<int> request){
  /*
   * Found and adapted from this source
   *
   * https://github.com/iamrohitsuthar/SPOS/blob/master/Bankers_Algorithm/BankersImplementation.java
  */

  int count = 0;
  bool execute = true;

  size_t i = 0, j = 0;
  std::vector<bool> visited(this->processes(), false);
  EVec::extended_vector_t<int> work = this->available_funds;

  while(count < this->processes()) {
    bool flag = false;
    for(i = 0; i < this->processes(); ++i) { // look at all the processes available
      execute = true;
      if(!visited[i]) {
        if(this->customers[i]->request() > work) { execute = false; break; } // if the request exceeds the funds allocated temporarily
        else {
          work+=this->customers[i]->get_init();
          this->safe_sequence[count++] = i;
          visited[i] = true;
          flag = true;
        }
      }
    }
    if(!flag){ break; }
  }
  return (count < this->processes())
             ? false
             : true;  // if we didn't go through all the possible processes
                      // (because one has failed), therefore we are not eligible
}

bool banker_t::is_available(EVec::extended_vector_t<int> request){ return this->available_funds > request; }

bool banker_t::can_grant_request(int index, EVec::extended_vector_t<int> request){
  /*
   * NOTE
   *
   * Algorithm taken from the lecture slides
  */

  if(request > this->available_funds){ return false; }
  return this->is_safe(index, request);
}

void banker_t::withdrawl_resources(customer_t* customer) {
  /*
   * Give resources to the customer
  */

  try{ 
    this->available_funds = this->overdraft_check(customer->request());
    customer->obtain_resources();
    change_value(&ALLOCATION_STATUS, true);
  } catch(std::exception& e) {
    change_value(&ALLOCATION_STATUS, false);
  }
}

void banker_t::deposit(customer_t* customer) {

  /*
   * Take all resources from customer
  */

  this->available_funds+=customer->get_maximum();
  customer->drop_resources();
}

EVec::extended_vector_t<int> banker_t::overdraft_check(EVec::extended_vector_t<int> request) {
  /*
   * Ensure there is no overdrafting from the resource pool
  */

  EVec::extended_vector_t<int> resultant(request.size(), 0);
  for(size_t i = 0; i < request.size(); ++i) {
    int _r = this->available_funds[i] - request[i];
    if(_r < 0) {
      throw std::logic_error("overdraft occurred!");
    } // overdraft occurs
    resultant[i] = _r;
  }
  return resultant;
}

bool banker_t::conduct_simulation(bool pedantic) {
  printf("[INFO] Conducting the simulation...\n");

  pthread_t tid[customers.size()];

  pthread_attr_t attr;
  pthread_mutexattr_t mutex_attr;
  pthread_attr_init(&attr);
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutex_init(&mutex_, &mutex_attr);
  
  // Update the global banker_t object
  // We need these to be available to the runner function
  // I could have passed a structure pointer to the runner function with all
  // the necessary objects but that ended in a big ball of fire

  banker_.finished = std::vector<bool>(this->customers.size(), false);
  banker_ = *this;

  // Create Threads
  int i = 0;
  for(auto customer : this->customers) {
    pthread_create(&tid[i++], &attr, runner, (void*)(customer_t*)customer);
  }

  // Join Threads
  i = 0;
  for(auto customer : this->customers) {
    pthread_join(tid[i++], NULL);
  }
  std::cout << banker_ << std::endl;

  return STATUS;
}

void banker_t::update_avaialble_funds(EVec::extended_vector_t<int> container) {
  this->available_funds = container;
}

void banker_t::add_customers(std::vector<customer_t*> container) {
  this->customers = container;
}

int banker_t::resources() const { return this->_m_resources; }
int banker_t::processes() const { return this->_n_processes; }

void banker_t::clear_sequence() { this->safe_sequence.clear(); }

std::vector<customer_t*> banker_t::get_customers() const{
  return this->customers;
}

std::ostream& operator<<(std::ostream& os, const banker_t& banker){
  os << "Available: ";
  /*
   * FIXME | well this is a fundamental problem that can't be fixed easily
   * the << operator freaks out when I try to define the function as a template
   * Unsure why this is the case but since this project is due soon and I am very tired
   * please settle for this function that just prints out the std::vector that controls this function
  */

  print_vector(banker.available_funds.get_data());
  os << std::endl;

  os << "ALLOCATED\tMAXIMUM\t\tNEED" << std::endl;
  
  for(auto customer : banker.get_customers()) {
    os << *customer; 
  }
  return os;
}
