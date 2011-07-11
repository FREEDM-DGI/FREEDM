//////////////////////////////////////////////////////////
/// @file         ReadMe
///
/// @author       Ravi Akella <rcaq5c@mst.edu>
///
/// @compiler     C++
///
/// @project      FREEDM DGI
///
/// Suggested modifications or questions about these codes
/// can be directed to Dr. Bruce McMillin, Department of
/// Computer Science, Missouri University of Science and
/// Technology, Rolla, 
/// MO  65409 (ff@mst.edu).
///
/////////////////////////////////////////////////////////

The current code is a Broker implementation of FREEDM DGI with
integrated Group Management and Load Balancing. 

The code for various modules can be located in respective directories labelled 
"gm" (for Group Management), "lb" (for LoadBalancing) and "sc" (for StateCollection) 

Mechanism for the propagation of group membership list to modules has been developed 
for the LoadBalancing as follows: Whenever a new group is formed, the new leader of the 
group packs the UUIDs of the members in its group as a string and sends it to loadbalancing 
module of its members including itself. The loadbalancing module, on receiving this list 
unpacks the list and adds up the UUIDs. Similar mechanism can also be extended to the state 
collection module.

GroupManagement:
--------------
The specific messages used are :

new_AreYouCoordinator();
new_Invitation();
new_Ready();
new_Response(std::string msg,std::string type);
new_Accept();
new_AreYouThere();
peer_list();

* Needs Improvement

LoadBalancing:
--------------

1) Use the "State.txt" file in folder named "lb" to perform experiments with a
different order of state changes.

2) The specific messages used are :
peerList - sent by Group Management module of the current leader 
request  - sent by a node in SUPPLY state to nodes in DEMAND
demand   - broadcasted by a node in DEMAND state to all members in group
normal   - broadcasted by a node in NORMAL state to all members in group
drafting - sent by a node in SUPPLY state in response to the 'yes' of a DEMAND node
         - the DEMAND node can send a  'no' in response to the request by a SUPPLY node
accept   - sent by a node in DEMAND state to indicate its readiness to involve in migration
load     - sent by the StateCollection module as ("lb.load") whenever it  needs to collect 
           the load status of its peers. LoadBalancing module responds by sending a message
           of the form ("statecollection.load") to the requesting source

* Needs Improvement           

StateCollection:
----------------

* Needs Improvement

