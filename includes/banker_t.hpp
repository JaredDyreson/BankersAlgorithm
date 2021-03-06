#pragma once

#include <vector>
#include <pthread.h>
#include <tuple>

#include "../includes/customer_t.hpp"
#include "../includes/extended_vector_t.hpp"

class banker_t {
  public:
    banker_t(EVec::extended_vector_t<int>, std::vector<customer_t*>);
    banker_t();

    EVec::extended_vector_t<int> get_available_funds();

    bool can_grant_request(int, EVec::extended_vector_t<int>);
    bool is_safe(int, EVec::extended_vector_t<int>);

    void update_avaialble_funds(EVec::extended_vector_t<int>);

    bool conduct_simulation(bool);
    void withdrawl_resources(customer_t*);
    void deposit(customer_t*);
    bool is_available(EVec::extended_vector_t<int>);

    EVec::extended_vector_t<int> overdraft_check(EVec::extended_vector_t<int>);

    std::vector<customer_t*> get_customers() const;

    void add_customers(std::vector<customer_t*>);

    int resources() const;
    int processes() const;
    void clear_sequence();

    friend std::ostream& operator<<(std::ostream&, const banker_t&);

  private:
    EVec::extended_vector_t<int> available_funds;
    EVec::extended_vector_t<int> total_allocated;
    
    std::vector<customer_t*> customers;
    std::vector<bool> finished;
    int _m_resources, _n_processes;
    EVec::extended_vector_t<int> safe_sequence;
};

