#include "abstractFactory.h"
#include "abstractModule.h"

class LedFactory : public AbstractFactory {
public:
    AbstractModule* createModule() const override {
        // return new LedModule();
    }
};