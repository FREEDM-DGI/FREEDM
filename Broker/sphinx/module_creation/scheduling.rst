.. _scheduling:

Scheduling DGI Modules
======================

DGI modules are scheduled through the Broker.
The broker provides a real-time round robin scheduler that manages the execution time of modules.
Modules can request timers, special objects that request a task be performed at some point in the future.
The Broker allows each module a certain amount of execution time each round.
A modules execution time is usually refered to as a `phase`.
Each module has it's own phase.
This line of code from `PosixMain.cpp` registers the length of a module's phase with the Broker::

    CBroker::Instance().RegisterModule("vv", boost::posix_time::milliseconds(2000));

In this case, our Volt Var module is allotted 2000 milliseconds of execution time each round.
We also invoked a call to Run() in `PosixMain.cpp` that is the entry point for our module's execution.
In order for anything else to happen though, we need to learn how to schedule tasks for our module to perform.

There are two options for scheduling tasks.
Tasks can be scheduled for immediate execution, they will be executed as soon as the active phase is your module.
If they are scheduled while the module is active, they will be immediately executed.
Tasks can be scheduled fo the future by using a timer.
They will execute when timer expires and the active phase is your module.

Scheduling For Immediate Execution
----------------------------------

The first approach to scheduling tasks is to schedule them for immediate execution.
These tasks will be run immediately while the active phase is your module.

Tasks are scheduled as functors.
The functor that is scheduled to execute must take no arguements and return void.
To do this, you will use `boost::bind` to create functors for the member functions of your module::

    // New Method
    void VVAgent::MyScheduledMethod()
    {
        Logger.Error<<"Schedule!"<<std::endl;
        CBroker::Instance().Schedule("vv",
            boost::bind(&VVAgent::MyScheduledMethod, this));
    }

    // Modified Existing method
    void VVAgent::Run()
    {
        CBroker::Instance().Schedule("vv",
            boost::bind(&VVAgent::MyScheduledMethod, this));
    }

Now, if we run the DGI, when the Volt Var module is active, it will repeatedly print "Schedule!" to the screen.
One thing to consider when programming for the DGI, is that modules are responsible for ensuring they do not exceede their alotted execution time.

