#include "../includes/banker_t.hpp"
#include "../includes/customer_t.hpp"
#include "../includes/extended_vector_t.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#define MUTEX_SAFE(x) pthread_mutex_lock(&mutex_); \
                      x; \
                      pthread_mutex_unlock(&mutex_);

#define DEADLOCK 100

pthread_mutex_t mutex_;
banker_t banker_;

void* runner(void* parameters) {
  customer_t* customer = (customer_t*)parameters;

  int i = 0;

  MUTEX_SAFE(printf("[INFO] Customer thread p#%d has started..\n",
                    customer->get_number()))

  while (!customer->needs_met() && i < DEADLOCK) {
    int index = customer->get_number();
    bool approved = banker_.can_grant_request(index, customer->get_request());

    if (approved) {
      MUTEX_SAFE(printf(
          "[APPROVED] Granting process %d the desired resources\n", index))
      banker_.withdrawl_resources(customer);

      MUTEX_SAFE(std::cout << banker_ << std::endl)

      if (customer->needs_met()) {
        MUTEX_SAFE(
            printf("[RECIEVED] Process %d has been sataiated and will give its "
                   "resources back\n",
                   index))
        banker_.deposit(customer);
      }

    } else {
      MUTEX_SAFE(printf(
          "[DENIED] Will not grant process %d desired resources\n", index))
    }
    ++i;
  }

  MUTEX_SAFE(printf("[INFO] Customer thread p#%d has completed..\n",
                    customer->get_number());)
  if (i < DEADLOCK) {
    pthread_exit(EXIT_SUCCESS);
  } else {
    std::cerr << "[FATAL] Algorithm has entered a deadlocked state!"
              << std::endl;
    pthread_exit(
        (void*)EXIT_FAILURE);  // uhhh what's going on clang, are you drunk?
  }
}

banker_t::banker_t(EVec::extended_vector_t<int> container, std::vector<customer_t*> customers) {
  this->available_funds = container;
  this->customers = customers;
}

banker_t::banker_t(){
  this->available_funds = EVec::extended_vector_t<int>();
  //this->customers = &std::vector<customer_t*>();
}

EVec::extended_vector_t<int> banker_t::get_available_funds(){
  return this->available_funds;
}

// FIXME : same as constructor
void banker_t::update_avaialble_funds(EVec::extended_vector_t<int> container) {
  this->available_funds = container;
}

template <typename T>
bool all(std::vector<T> container){
  for(bool element : container){
    if(!element){ return false; }
  }
  return true;
}

template <typename T>

bool any_(std::vector<T> container) {
  int i = 0;
  for(auto element : container) {
    if(element){ i++; }
  }
  return (i > 0);
}

bool banker_t::can_grant_request(int index, EVec::extended_vector_t<int> request){
  /*
   * NOTE
   *
   * Algorithm taken from the lecture slides
  */

  size_t _n_procs = this->customers.size(); // number of processes
  // size_t _m_resources = this->customers[0]->get_maximum().size(); // number of resources

  // Vector that notes if each process would have finished if given the proper resources

  EVec::extended_vector_t<int> work = this->available_funds;

  // Check the system to see if it's in a safe state
  for(size_t i = 0; i < _n_procs; ++i) {
    // Current need of the process
    EVec::extended_vector_t<int> need_i = this->customers[i]->get_request();
    // Currently allocated resources
    // This vector gets updated if the process is given any resources
    EVec::extended_vector_t<int> allocation_i = this->customers[i]->get_init();

    // If the resource has a smaller resource footprint, then we can allow it
    // And if it does not exceed the available funds
    if (!this->finished[i] && need_i <= work) {
      //  PRETEND TO ALLOCATE
      work += allocation_i;
      this->finished[i] = true;
    } 
    else {
      // The process should automatically be denied resources
      // as it would put the system in an unsafe state
      return false;
    }
  }
  // if all processes will result in a safe state
  // we are okay
  bool a = any_(this->finished);
  this->finished = std::vector<bool>(this->customers.size(), false);
  return a;
}

void banker_t::withdrawl_resources(customer_t* customer) {
  /*
   * Give resources to the customer
  */

  this->available_funds-=customer->get_request();
  customer->obtain_resources();
}

void banker_t::deposit(customer_t* customer) {

  /*
   * Take all resources from customer
  */

  this->available_funds+=customer->get_maximum();
  customer->drop_resources();
}

void banker_t::conduct_simulation() {
  printf("[INFO] Conducting the simulation...\n");

  pthread_t tid[customers.size()];

  pthread_attr_t attr;
  pthread_mutexattr_t mutex_attr;
  pthread_attr_init(&attr);
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutex_init(&mutex_, &mutex_attr);
  
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
}


void banker_t::add_customers(std::vector<customer_t*> container) {
  this->customers = container;
}

std::vector<customer_t*> banker_t::get_customers() const{
  return this->customers;
}

std::ostream& operator<<(std::ostream& os, const banker_t& banker){
  os << "Available: ";
  // FIXME
  print_vector(banker.available_funds.get_data());
  os << std::endl;

  os << "ALLOCATED\tMAXIMUM\t\tNEED" << std::endl;
  
  for(auto customer : banker.get_customers()) {
    os << *customer; 
  }
  return os;
}
