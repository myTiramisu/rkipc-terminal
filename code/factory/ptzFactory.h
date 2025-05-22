#include "abstractFactory.h"
#include "abstractModule.h"

class PtzFactory : public AbstractFactory {
public:
    AbstractModule* createModule() const override {
        // return new LedModule();
    }
};