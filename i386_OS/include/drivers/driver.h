#ifndef __MYOS_DRIVERS_DRIVER_H
#define __MYOS_DRIVERS_DRIVER_H

namespace myos
{
    namespace drivers
    {
        class Driver
        {
        public:
            Driver();
            ~Driver();
            virtual void Activate();
            virtual int Reset();
            virtual void Deactivate();
        };

        /*Every object derived from the Driver class will have its own Activate , Reset and Deactivate functions so because of that we define these functions as virtual functions in the Driver class so that when calling them they will resolve to the most derived class .So for example both mouse and keyboard are derived from the Driver class and both have Activate functions but both functions do different things .*/


        class DriverManager
        {
        public:
            Driver* drivers[265];
            int numDrivers;
            
        public:
            DriverManager();
            void AddDriver(Driver*);
            void ActivateAll();
        };
    }
}

#endif
