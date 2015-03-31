.. _scheduling:

Scheduling DGI Modules
======================

`The CBroker (the broker) is in CBroker.hpp`

DGI modules are scheduled through the Broker.
The broker provides a real-time round robin scheduler that manages the execution time of modules.
Modules can request timers, special objects that request a task be performed at some point in the future.
The Broker allows each module a certain amount of execution time each round.
A modules execution time is usually refered to as a *phase*.
Each module has it's own phase.
This line of code from `PosixMain.cpp` registers the length of a module's phase with the Broker::

    CBroker::Instance().RegisterModule("vv", boost::posix_time::milliseconds(2000));

In this case, our Volt Var module is allotted 2000 milliseconds of execution time each round.
We also invoked a call to **Run()** in `PosixMain.cpp` that is the entry point for our module's execution.
In order for anything else to happen though, we need to learn how to schedule tasks for our module to perform.

There are two options for scheduling tasks.
Tasks can be scheduled for immediate execution, they will be executed as soon as the active phase is your module.
If they are scheduled while the module is active, they will be immediately executed.
Tasks can be scheduled fo the future by using a timer.
They will execute when timer expires and the active phase is your module.

Scheduling For Immediate Execution
----------------------------------

`boost::bind is a part of <boost/bind.hpp>`

The first approach to scheduling tasks is to schedule them for immediate execution.
These tasks will be run immediately while the active phase is your module.

Tasks are scheduled as functors.
The functor that is scheduled to execute must take no arguements and return void.
To do this, you will use **boost::bind** to create functors for the member functions of your module::

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
The scheduler does not actively enforce any timing requirements -- it is your job as the programmer to ensure your module does not exceed its alotted execution time.
Scheduling for immediate execution has the potential to do so if the time to complete the scheduled task exceeds the amount of time remainin in the module.
Exceeding the phase time for your module can cause faults in other modules, resulting in unpredictable behavior.

Scheduling Tasks To Run In The Future
-------------------------------------

`boost::posix_time::ptime is a part of <boost/date_time/posix_time/posix_time.hpp> and <boost/date_time/posix_time/posix_time_types.hpp>`

`boost::asio::error::operation_aborted and boost::system::error_code are a part of <boost/asio.hpp>`


Our second approach to scheduling tasks is to use timers to schedule them to be executed in the future.
In order to use timers, you must first ask the **Broker** to allocate a timer for you.
A timer can be used multiple times, but can only be used for one pending task at a time.
A good place to ask the Broker to allocate a timer is in your constructor::

    VVAgent::VVAgent()
    {
        m_timer = CBroker::Instance().AllocateTimer("vv");
    }

Where ``m_timer`` is a member variable of **VVAgent** of type **CBroker::TimerHandle**.
The arguement to the AllocateTimer method is the same short name that you used to register your module with the Broker.

`Important! Make sure the short name you use to allocate the timer matches the one used to register the module, or the task will never be executed.`

You should schedule anything in the constructor, instead you'll schedule your first tasks in the **Run()** method.
Here is a modification of the first example that runs our task every 300 milliseconds::

    void VVAgent::MyScheduledMethod(const boost::system::error_code& err)
    {
        if(!err)
        {
            Logger.Error<<"Schedule!"<<std::endl;
            CBroker::Instance().Schedule(m_timer, boost::posix_time::milliseconds(300),
                boost::bind(&VVAgent::MyScheduledMethod, this, boost::asio::placeholders::error));
        }
        else
        {
            /* An error occurred or timer was canceled */
            Logger.Error << err << std::endl;
        }

    }

    void VVAgent::Run()
    {
        CBroker::Instance().Schedule("vv",
            boost::bind(&VVAgent::MyScheduledMethod, this, boost::system::error_code()));
    }

Methods that are scheduled with a timer must expect one argument of type **boost::system::error_code**.
This argument will be populated with the reason why the timer expired.
If the value is zero, the timer expired because the specified amount of time had passed.
If the value is non-zero, the timer expired either because it was cancelled or some other system error has occured.
A timer can be cancelled if the timer is used to schedule another task before it expires, or if the DGI is stopping.
If a timer is cancelled, err will have the value ``boost::asio::error::operation_aborted``
It is a good practice to check err and only schedule new tasks if has expired for a reason you expect.
In this example we only schedule our task again if the timer was not cancelled.

Inside **VVAgent::MyScheduledMethod** we call the **Broker::Schedule** function to schedule our method to run again.
The first parameter ``m_timer`` is the timer we allocated in the constructor.
The second parameter is the amount of time, as a **boost::posix_time::time_duration** that should pass before the task is executed.
The third parameter is a functor that will be executed when the timer expires.
This functor expects one argument, denoted by **boost::asio::placeholders::error** that will be filled with a **boost::system::error_code** when the timer expires.

The **Run()** method demonstrates how a method that is normally called by a timer can be scheduled to run immediately.
In the example, the call to **boost::bind** that creates the functor, now takes another argument **boost::system::error_code** that binds a zero error code to the eventual method call.

If a timer expires while the module is not active (that is, it is another module's phase), the execution of the method will be delayed until that module is active.

Scheduling Tasks To Run Next Time
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`boost::posix_time::not_a_date_time is a part of <boost/date_time/posix_time/posix_time.hpp>`

Tasks can also be scheduled to run the next time a module is active.
Tasks scheduled this way still need a timer, but the timer will expire as soon the module is no longer active.
Here is a modification of our previous example that will execute **MyScheduledMethod** once each round::

    void VVAgent::MyScheduledMethod(const boost::system::error_code& err)
    {
        if(!err)
        {
            Logger.Error<<"Schedule!"<<std::endl;
            CBroker::Instance().Schedule(m_timer, boost::posix_time::not_a_date_time,
                boost::bind(&VVAgent::MyScheduledMethod, this, boost::asio::placeholders::error));
        }
        else
        {
            /* An error occurred or timer was canceled */
            Logger.Error << err << std::endl;
        }

    }

    void VVAgent::Run()
    {
        CBroker::Instance().Schedule("vv",
            boost::bind(&VVAgent::MyScheduledMethod, this, boost::system::error_code()));
    }

From the previous example, we have replaced the 300 millisecond **boost::posix_time::time_duration** with a ``boost::posix_time::not_a_date_time``.
Now when the **VVAgent**'s phase ends, ``m_timer`` will expire, and when it is the module's phase again, **MyScheduledMethod** will be executed.

From Here, you can read more about the scheduler: :ref:`cbroker`

Or you can go on to message passing: :ref:`receiving-messages`
