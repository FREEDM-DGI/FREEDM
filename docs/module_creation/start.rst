Starting Your Module
====================

IDGIModule Reference
^^^^^^^^^^^^^^^^^^^^

DGI modules are always based on the abstract interface IDGIModule.

`IDGIModule is found in IDGIModule.hpp`

.. doxygenclass:: freedm::broker::IDGIModule
    :project: FREEDM
    :members:
    :protected-members:
 
Additionally, you will want to create a Run() method that will kick-off the actions your module peforms.

Module Creation
^^^^^^^^^^^^^^^

To create your module, first create a new directory for it in the `Broker/src` directory. You should select a short name for your module (typically 3 characters or less) use that as the name of your folder. For example, if you are creating a Volt-Var control module you might start your module like so::

$ cd Broker/src
$ mkdir vv
$ cd vv
$ touch VoltVar.cpp
$ touch VoltVar.hpp

This will create a folder for your module and start you out with two blank C++ files where you will create your module.


Module .hpp
^^^^^^^^^^^
The .hpp should contain your Module's class declaration. Modules inherit from IDGIModule. 
In our example where we are creating `VoltVar.hpp`, this will get us started::

    #ifndef VOLTVAR_HPP_
    #define VOLTVAR_HPP_

    #include "IDGIModule.hpp"
    #include "CPeerNode.hpp"
    #include "PeerSets.hpp"

    #include "messages/ModuleMessage.pb.h"

    namespace freedm {
    namespace broker {
    namespace vv {

    /// Declaration of Garcia-Molina Invitation Leader Election algorithm.
    class VVAgent
      : public IDGIModule
    {
      public:
        /// Constructor for using this object as a module.
        VVAgent();
        /// Module destructor
        ~VVAgent();
        /// Called to start the system
        int	Run();
      private:
        /// Handles received messages
        void HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer);
    };

    } // namespace vv
    } // namespace broker
    } // namespace freedm
    
We've created a **constructor**, **destructor**, **Run()** method, and a method for handling messages (**HandleIncomingMessages()**).
Let's implement these methods in `VoltVar.cpp`.

Module .cpp
^^^^^^^^^^^

Here are the implementations for the methods we've defined so far::

    #include "VoltVar.hpp"
    #include "CBroker.hpp"
    #include "CLogger.hpp"

    namespace freedm {
    namespace broker {
    namespace vv {

    namespace {
        /// This file's logger.
        CLocalLogger Logger(__FILE__);
    }
    
    VVAgent::VVAgent()
    {
    }
    
    VVAgent::~VVAgent()
    {
    }
    
    int VVAgent::Run()
    {
        Logger.Warn<<"Volt Var Control Sure Is Neat!"<<std::endl;
    }
    void VVAgent::HandleIncomingMessage(boost::shared_ptr<const ModuleMessage> msg, CPeerNode peer)
    {
        Logger.Warn<<"Dropped message of unexpected type:\n" << msg->DebugString();
    }
    
    } // namespace vv
    } // namespace broker
    } // namespace freedm

What's going on here? We've created an instance of **CLocalLogger** called ``Logger``. This allows us to log messages from this module. When creating your module you may find it handy to familiarize yourself with :ref:`reference-logger`. 

Next, we need to register our module with the scheduler and message delivery system. In `Broker/src/PosixMain.cpp` locate the initialize modules section and add your new module::

    // Initialize modules
    boost::shared_ptr<IDGIModule> GM = boost::make_shared<gm::GMAgent>();
    boost::shared_ptr<IDGIModule> SC = boost::make_shared<sc::SCAgent>();
    boost::shared_ptr<IDGIModule> LB = boost::make_shared<lb::LBAgent>();
    
    // My new module!!
    boost::shared_ptr<IDGIModule> VV = boost::make_shared<lb::VVAgent>();
    
Just below that you'll register your module with the dispatcher, which is responsible for delivering messages to your module::

    // Instantiate and register the group management module
    CBroker::Instance().RegisterModule("gm",
        boost::posix_time::milliseconds(CTimings.Get("GM_PHASE_TIME")));
    CDispatcher::Instance().RegisterReadHandler(GM, "gm");
    // Instantiate and register the state collection module
    CBroker::Instance().RegisterModule("sc",
        boost::posix_time::milliseconds(CTimings.Get("SC_PHASE_TIME")));
    CDispatcher::Instance().RegisterReadHandler(SC, "sc");
    // StateCollection wants to receive Accept messages addressed to lb.
    CDispatcher::Instance().RegisterReadHandler(SC, "lb");
    // Instantiate and register the power management module
    CBroker::Instance().RegisterModule("lb",
        boost::posix_time::milliseconds(CTimings.Get("LB_PHASE_TIME")));
    CDispatcher::Instance().RegisterReadHandler(LB, "lb");
    
    // REGISTER YOUR NEW MODULE
    CBroker::Instance().RegisterModule("vv", boost::posix_time::milliseconds(2000));
    CDispatcher::Instance().RegisterReadHandler(VV, "vv");
    
What did we do here? I've registered our module with the **Broker**, which will allocate it 2000 milliseconds of execution time in the real time scheduler.
Later, when we start working with the schedule in our module, we'll cover adding entries to the timing configuration file, so that users can adjust the timing of your module for their system.
Next, we will need to invoke a call to our **Run()** method to get our module going::

    Logger.Debug << "Starting thread of Modules" << std::endl;
    CBroker::Instance().Schedule(
        "gm",
        boost::bind(&gm::GMAgent::Run, boost::dynamic_pointer_cast<gm::GMAgent>(GM)),
        false);
    CBroker::Instance().Schedule(
        "lb",
        boost::bind(&lb::LBAgent::Run, boost::dynamic_pointer_cast<lb::LBAgent>(LB)),
        false);
    
    // New Module!
    CBroker::Instance().Schedule(
        "vv",
        boost::bind(&lb::VVAgent::Run, boost::dynamic_pointer_cast<lb::VVAgent>(VV))
        false);

When the broker starts, the Volt Var module's **Run()** method will be called. However, before we run DGI with our new module, we need to add our new module to the CMake configuration. Edit `Broker/src/CMakeLists.txt` and add your new module::

    ...
    CClockSynchronizer.cpp
    CTimings.cpp
    CPhysicalTopology.cpp
    gm/GroupManagement.cpp
    lb/LoadBalance.cpp
    sc/StateCollection.cpp
    vv/VoltVar.cpp
    )
    
Then to build, you will invoke ``cmake`` and then ``make``::

    $ pwd
    /home/scj7t4/FREEDM/Broker
    $ cmake
    $ make

If everything goes well, you can run `PosixBroker`. With careful observation you should be able to catch the message we log in the **Run()** method of our module::

    2015-Feb-17 13:10:50.014181 : VoltVar.cpp : Warn(3):
        Volt Var Control Sure Is Neat!

Next, let's make our module do something go to :ref:`scheduling`
