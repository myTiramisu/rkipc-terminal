#include "abstractFactory.h"
#include "abstractModule.h"

class DisplayFactory : public AbstractFactory {
public:
    AbstractModule* createModule() const override {
        // return new LedModule();
    }
};