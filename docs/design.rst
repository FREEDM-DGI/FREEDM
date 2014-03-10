========================
 General Approach
========================

Program Flow
------------------------

PosixMain.cpp -> main()
	- Sets up all software modules (LeaderElection, StateCollection, etc.)
	- Create BrokerServer
	- Register each software module with broker
		- This registers XML portion of ptree that this module is responsible for
		- Registers callbacks to handle these sections of a ptree
	- Run Broker
		- listen() on network port.
		- waits for incoming connections

On new connection,
	- spawns a new CConnection object, adds it to ConnectionManager
	- register read callback of connection to broker HandleRead [i.e. CConnection::OnRead -> Broker::HandleRead ]

Upon read,
	- CConnection::HandleRead is activated
	- Message is parsed into ptree
	- Broker::HandleRead is called on ptree [XXX NEEDS CCONNECTION OBJECT TOO!! XXX]
	- Registered handlers are called on ptree

Software modules may also register write handlers. This is needed for StateCollection. Handler will be called on all outgoing messages.
