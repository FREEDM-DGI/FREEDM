.. _physical-topology:

Physical Topology
=================

DGI can react to changes in the physical topology if the PhysicalTopology code is enabled. Currently, this code is only available in the physical topology side-branch. It should be a part of master (and a release) soon.

Physical Topology is based on vertices (SSTs) and edges. Edges can have 0 or more FIDs. These FIDs determine the availability of the edge: all FIDs on the edge must be closed (Unless the edge does not have an FID). If there is no
series of edges which connect two SSTs, they should not be in a group together.

Physical Topology Configuration
-------------------------------

Topology is configured in ``config/topology.cfg``. A topology config file looks like this::

    edge a b
    edge b c
    edge c a
    sst a raichu.freedm:1870
    sst b manectric.freedm:1870
    sst c galvantula.freedm:1870
    fid a b FID1
    fid a b FID4
    fid b c FID2
    fid c a FID3

Each line of the file is composed of a statement type, and then a series of keywords that are necessary to construct that object.

* **edge** - A physical connection between two SSTs. An edge indicates there is a direct physical connection between two SSTs through a power line or similar object. An edge is followed by strings that represent the two verticies they connect. For convenience, the DGI controlling the vertex is set by the _sst_ statement. Only one edge for each vertex pair needs to be named, and all edges are bidirectional by default (that is, ``edge a b`` also gives you ``edge b a``)
* **sst** - A vertex. This statement is followed by two strings: the first is the name of the vertex. This is the name used in the ``edge`` and ``fid`` statements. The second string is the UUID of the DGI that controls that vertex.
* **fid** - A control for an edge. This statement is controlled by three strings: first two strings are the edge that this FID controls, which is named in the same was as the edge above, which two vertex names. The third string is the name of the device which controls this edge. A device can control multiple edges and multiple devices can control one edge.

The topology configuration file is specified by adding the ``topology-config` option to ``freedm.cfg``. For example, this line in a ``freedm.cfg`` enables physical topology::

    topology-config=config/topology.cfg

The topology configuration file should be the same on all DGI peers.

Expected Group Management Behavior
----------------------------------

If there is no topology configuration file specified in `freedm.cfg` then the physical topology feature is disabled and
DGI will group with all available peers.

If a topology configuration file is specified then the DGI will only group with nodes that it deems to be reachable. If the FID state changes so that a node is no longer reachable then DGI will remove that peer from the group. The peer does not immediately receive the notification it has been removed so it will appear to remain in the group for an additional round; however, no other DGI will respond to its requests (so no migrations will occur) and it will leave during the next Group Management phase.

Physical Topology Implementation
--------------------------------

The FID state is passed to peers using the "Are You Coordinator" (AYC) response message: The message will be received by the coordinator from any node that wants to merge groups and any node that wants to remain in a group with that coordinator. The coordinator combines all the reported FID states with its own and then runs a breadth first search (BFS) on the specified topology. Any edge where a controlling FID is marked as open is not used to determining physical reachability. The BFS returns a set of peers it has determined to be connected to the coordinator. The coordinator then interacts with the reachable nodes, ignoring any peers that are not reachable. Each round a peer may provide new FID state to show that they may now be reachable. The held state is wiped each time the BFS is run: an edge is only held open if the FIDs controlling it are consistently reported to coordinator.