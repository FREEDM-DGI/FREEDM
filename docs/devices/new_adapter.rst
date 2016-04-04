Creating a New Adapter
======================

If none of the adapter types provided by the DGI are sufficient for communication with a particular device, a new adapter can be implemented in the DGI without much code modification. However, this requires extreme expertise in both C++ and the BOOST libraries and is not recommended for most users. This tutorial will cover the basics on how to create a new adapter class.

Required Functions
------------------

All adapters must inherit from the base adapter class :cpp:class:`IAdapter` located at ``Broker/src/device/IAdapter.hpp``. IAdapter has a required set of functions that all adapter classes must implement. These functions are:

.. cpp:function:: void IAdapter::Start()

The start function is called after a new instance of the adapter has been created and the DGI is ready to use the new adapter to communicate with physical devices. This function is guarenteed to be called exactly once by the DGI, and the adapter is expected to do no work until after this function has been called. This function should be implemented to start the protocol that sends and receives device data.

.. cpp:function:: void IAdapter::Stop()

The stop function is called when the DGI terminates to allow the adapter a chance to cleanly terminate its connection with its associated physical devices. This function will be called once during the DGI teardown procedure. At the end of the function call, the DGI should be disconnected from the device.

.. cpp:function:: float IAdapter::GetState(const std::string, const std::string) const

The GetState function is called each time a virtual device within the DGI attempts to access the current state of its real device communicating with the adapter. Each adapter is required to ensure that the GetState function returns the most recent value of a requested state each time it is called. However, there is no guarentee that this function will ever be called.

.. cpp:function:: void IAdapter::SetCommand(const std::string, const std::string, const float)

The SetCommand function is called each time a virtual device within the DGI attempts to send a command to its real device communication with the adapter. Each adapter must ensure that the value sent with this function call eventually reaches the physical device unless a later SetCommand call changes the commanded value. There is no guarentee that this function will ever be called, and as such the initial default values for each command must be set by the adapter itself during construction.

.. cpp:function:: static IAdapter::Pointer YourAdapter::Create(...)

The create function is not required in the sense it will not cause a compile error if omitted. However, a create function makes integration of new adapters with the DGI much easier and is thus strongly encouraged. The parameters of the create function will depend on which variables the adapter needs to be initialized, but all create functions should be static. This function will be called when a new instance of the adapter is first created.

Sample Header File
------------------

The following is a sample header file that provides a template most adapters should utilize. Both the run function and the deadline timer are optional features that should be utilized if your adapter has to periodically reschedule itself to maintain communication with the physical device. The remaining functions are mostly required as stated above.

::

    #include "IAdapter.hpp"

    #include <boost/enable_shared_from_this.hpp>
    #include <boost/shared_ptr.hpp>
    #include <boost/asio/deadline_timer.hpp>
    #include <boost/asio/io_service.hpp>

    namespace freedm {
    namespace broker {
    namespace device {

    class CYourAdapter
         : public IAdapter, public boost::enable_shared_from_this<CYourAdapter>
    {
    public:
        /// Typedef for ease of use.
        typedef boost::shared_ptr<CYourAdapter> Pointer;

        /// Called once when adapter is first created (recommended).
        static IAdapter::Pointer Create(boost::asio::io_service & service);

        /// Called once afted the adapter is initialized (required).
        void Start();

        /// Called once before the DGI terminates (required).
        void Stop();
    
        /// Called each time a DGI module tries to send a command (required).
        void SetCommand(const std::string device, const std::string signal, const SignalValue value);

        /// Called each time a DGI module tries to read a state (required).
        SignalValue GetState(const std::string device, const std::string signal) const;
    
        /// Destructor.
        ~CYourAdapter();
    private:
        /// Constructor.
        CYourAdapter(boost::asio::io_service & service);

        /// Called periodically to maintain communication with devices (optional).
        void Run(const boost::system::error_code & e);

        /// Used to schedule the Run function (optional).
        boost::asio::deadline_timer m_timer;
    };

    } //namespace device
    } //namespace broker
    } //namespace freedm

This template assumes that the adapter maintains constant communication with its associated physical devices. The ``void CYourAdapter::Run(const boost::system::error_code &)`` function will be scheduled at periodic intervals to send and receive data. This data will have to be stored in a member variable inside of the adapter (not in the template), and both the SetCommand and GetState functions will use the member variables instead of directly communicating with the device. This format is identical to the RTDS adapter which can be used as an additional example at ``Broker/src/device/CRtdsAdapter.hpp``.

Another possible implementation of adapters would be for the SetCommand and GetState functions to send messages to the device on demand as they are called. In this case, the Run function might be unnecessary as all the communication happens through the required function implementations. In this case, only the required functions would be necessary, and the Run function and its associated :cpp:type:`boost::asio::deadline_timer` could be omitted.

Sample Implementation File
--------------------------

The following is a sample implementation file that shows how the various functions interact with each other. It is mostly intended to illustrate how to set up a reoccuring Run function to maintain communication with physical devices. If the Run function is not relevant to your adapter type, this example can likely be ignored.

::

    #include "CYourAdapter.hpp"
    #include "CLogger.hpp"

    namespace freedm {
    namespace broker {
    namespace device {

    IAdapter::Pointer CYourAdapter::Create(boost::asio::io_service & service)
    {
      return CYourAdapter::Pointer(new CYourAdapter(service));
    }

    CYourAdapter::CYourAdapter(boost::asio::io_service & service)
      : m_timer(service)
    {
        // initialize your adapter here (change arguments as needed)
    }

    void CYourAdapter::Start()
    {
        // do post-initialization processing here
  
        // schedule Run 1000 milliseconds from now (change time as needed)
        m_timer.expires_from_now(boost::posix_time::milliseconds(1000));
        m_timer.async_wait(boost::bind(&CYourAdapter::Run, shared_from_this(), boost::asio::placeholders::error);
    }

    void CYourAdapter::Stop()
    {
        // do pre-termination processing here
    }

    CYourAdapter::~CYourAdapter()
    {
        // deconstruct your adapter here
    }

    void CYourAdapter::Run(const boost::system::error_code & e)
    {
        if(!e)
        {
            // do periodic communication with your devices here
            // this should be the main portion (if not all) of your code
    
            // reschedule Run 1000 milliseconds from now (change time as needed)
            m_timer.expires_from_now(boost::posix_time::milliseconds(1000));
            m_timer.async_wait(boost::bind(&CYourAdapter::Run, shared_from_this(), boost::asio::placeholders::error);
        }
        else if(e == boost::asio::error::operation_aborted)
        {
            // happens if DGI is terminating ; do nothing special
        }
        else
        {
            // error condition! something in the device framework is broken!
        }
    }

    void CYourAdapter::SetCommand(const std::string device, const std::string signal, const SignalValue value)
    {
        // send or prepare to send a command to your devices here
    }

    SignalValue CYourAdapter::GetState(const std::string device, const std::string signal) const
    {
        // read a state from your devices here
    }

    } //namespace device
    } //namespace broker
    } //namespace freedm

Assuming that the communication code has a sequential block of code that sends a block of data to a device and receives a block of data in return, this sequential code should all be placed into the run function with the data to be sent and receive declared as member variables. Then the SetCommand and GetState functions would write to and read from these member variables rather than interacting with the physical device. This is the approach of the RTDS adapter, which can be used as a sample implementation at ``Broker/src/device/CRtdsAdapter``.

Integration with the DGI
------------------------

Your new adapter class must be integrated with the DGI once its implementation has been completed. This involves three separate steps.

First, the adapter must be included in the DDGI compilation process. We assume that your adapter is located at *Broker/src/device/CYourAdapter.cpp*. Open the file ``Broker/src/device/CMakeLists.txt`` and locate the ``set(DEVICE_FILES`` command near the very top. Include ``CYourAdapter.cpp`` after *DEVICE_FILES* and before the closing parenthesis, following the example of the other device files. After this change, when ``make`` is run from the *Broker* directory, *CYourAdapter.cpp* will be included in the DGI compilation process.

Second, you must decide what configurable options are required for your adapter type.  All standard adapters (plug and play being the exception) are created through the adapter configuration file ``Broker/config/adapter.xml``. When the DGI starts with an adapter configuration file specified in ``Broker/config/freedm.cfg``, it parses the contents of the file to determine which adapters it needs to create. Your adapter will also be configured in *adapter.xml*. Consider the first line of each adapter configuration::

    <adapter name = "simulation" type = "rtds">

You must define a new ``type`` identifier for your adapter which will go in the type field when a user wants to create a new instance of your adapter type. You cannot remove the **<state>** and **<command>** subtags for your adapter specification, as they are required by the DGI to create virtual devices that modules will use to interact with your adapter. However, you can change the contents of the **<info>** subtag which is intended to contain all the configurable settings unique to your adapter. If you have any user-defined settings that are required when a new instance of your adapter is created, you should determine how best to incorporate them into this **<info>** tag in *adapter.xml*.

Third, you must modify the behemoth of a file that is CAdapterFactory.cpp. This file handles the creation and maintenance of all types of adapters, including the parsing of the adapter configuraton file mentioned above. It can be found at ``Broker/src/device/CAdapterFactory.cpp``. The most relevant functions for creation of a new adapter are the ``void CAdapterFactory::CreateAdapter(const boost::property_tree::ptree &)`` function and the ``void CAdapterFactory::InitializeAdapter(IAdapter::Pointer, const boost::property_tree::ptree &)`` function. The CreateAdapter function is called each time a new **<adapter>** tag is parsed, and the :cpp:type:`boost::property_tree::ptree` stores the contents of the XML specification for the new adapter. The InitializeAdapter function is called once for each adapter, and parses the contents of the **<state>** and **<command>** specifications.

For the DGI to create your adapter, it must create an instance of your adapter's type when it parses the XML in the CreateAdapter function. Locate the following line of code::

    if( type == "rtds" )
    {
        adapter = CRtdsAdapter::Create(m_ios, subtree);
    }
    else if( type == "pnp" )
    {
        adapter = CPnpAdapter::Create(m_ios, subtree, m_server->GetClient());
    }
    else if( type == "fake" )
    {
        adapter = CFakeAdapter::Create();
    }
    else
    {
        throw EDgiConfigError("Unregistered adapter type: " + type);
    }

This conditional determines which type of adapter the **<adapter>** tag specifies and creates the appropriate adapter class in the DGI. You must extend this conditional to support your new adapter type::

    else if( type == "YourAdapter" )
    {
        adapter = CYourAdapter::Create(m_ios);
    }

This will call the ``static IAdapter::Pointer CYourAdapter::Create(...)`` function you defined in your adapter implementation, and should be passed all the parameters that are required for your implementation's constructor. For the other adapter types, not that several of them are passed the variable *subtree* which is a :cpp:type:`boost::property_tree::ptree`. This stores the contents of the **<info>** tag from the *adapter.xml* file. If your adapter uses the **<info>** tag, you should also pass this subtree variable. For using the *subtree* variable, you will have to refer to the `BOOST Property Tree Documentation <http://www.boost.org/doc/libs/1_57_0/doc/html/property_tree.html>`_ as well as the ``Broker/src/device/CRtdsAdapter.cpp`` adapter implementation for an example.

With this, your adapter has been compiled into the DGI, constructed when the DGI parses the adapter configuration file, and perhaps initialized with the contents of its **<info>** tag. However, at no point does your adapter learn of the devices that have been attached to it. The **<state>** and **<command>** tags from the adapter configuration file are never seen by your adapter instance. These tags are instead parsed in the function ``void CAdapterFactory::InitializeAdapter(IAdapter::Pointer, const boost::property_tree::ptree &)``. If your adapter needs to know the type and number of devices the DGI has associated with it, you must modify this InitializeAdapter function.

First, search for the following line::

    IBufferAdapter::Pointer buffer = boost::dynamic_pointer_cast<IBufferAdapter>(adapter);

You will want to add an additional line after this one with a similar format for your adapter::

    CYourAdapter::Pointer youradapter = boost::dynamic_pointer_cast<CYourAdapter>(adapter);

This will attempt to convert the :cpp:type:`IAdapter::Pointer` passed to the function into a :cpp:type:`CYourAdapter::Pointer`. If the conversion fails, the variable youradapter will store a null pointer. This conversion is required because any modifications you make to this function are unique to your adapter. They should not be executed if the adapter passed to this function does not have your type, and this dynamic pointer cast provides a very easy way to determine if the adapter has your type in conditionals.

Next search for the line::

    // create the device when first seen
    if( devtype.count(name) == 0 )
    {
        CreateDevice(name, type, adapter);
        adapter->RegisterDevice(name);
        devtype[name]  = type;
        states[name]   = 0;
        commands[name] = 0;
    }

This block of code executes when a device is seen for the first time in the *adapter.xml* configuration file, and creates a virtual device for use in the DGI. The name of this device is already associated with your adapter through the ``void IAdapter::RegisterDevice(std::string)`` function call, and if you only need to know the number of devices you can overwrite this function in your implementation file. However, by default, the type is not sent to your adapter. Change the code to the following::

    // create the device when first seen
    if( devtype.count(name) == 0 )
    {
        CreateDevice(name, type, adapter);
        adapter->RegisterDevice(name);
        devtype[name]  = type;
        states[name]   = 0;
        commands[name] = 0;

        // use the dynamic pointer cast from before
        if( youradapter )
        {
            // if the code executes this, the adapter is of type CYourAdapter
            youradapter->RegisterType(type);
        }
    }

With this modification, ``void CYourAdapter::RegisterType(std::string)`` will be called once for each device associated with your adapter in the *adapter.xml* specification file. However, you will have to implement the RegisterType function as it is not a standard adapter function.

This tutorial only provides a brief overview of creation of a new adapter type, as well as several possibilities for integrating the new type with the DGI. The creation of a new adapter type is complicated and requires extensive knowledge of the device architecture. If you need to create a new adapter type, we strongly recommend you contact the DGI development team and keep in close contact with us. However, it should not be necessary to look at any part of the code other than the two CAdapterFactory functions mentioned in this section.
