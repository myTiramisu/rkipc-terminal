#include "observer.h"
#include "publisher.h"
#include <iostream>


Observer::Observer(Publisher* publisher, std::string name)
    : m_publisher(publisher), m_name(name) {
    m_publisher->attach(this);
}

Observer::~Observer() {
    std::cout << "Observer " << m_name << " is destroyed" << std::endl;
}

void Observer::unsubscribe() {
    m_publisher->detach(this);
}
