DGI Features
============

The DGI comes with everything needed to implement distributed algorithms for power device control in the smart grid.

* Real-time scheduling for execution of distributed algorithms using an integrated round-robin scheduler (:ref:`cbroker`)
* Automatic detection and configuration for DGI processes. The DGI automatically manages groups. Every module receives a list of available peers to use for executing algorithms. Updates are pushed to each module on change. (:ref:`group-management`)
* Device management and integration with PSCAD and RSCAD/RTDS simulations (:ref:`rtds-adapter`)
* Physical devices can be easily integrated by implementing our Plug n' Play protocol (:ref:`pnp-adapter`)
* Casually consistent global snapshot capturing. This can be used to capture the state of a smart-grid using a method that compensates for latency. (:ref:`state-collection`)